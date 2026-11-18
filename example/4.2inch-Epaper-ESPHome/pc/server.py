from __future__ import annotations

import asyncio
import json
import logging
import math
import os
from pathlib import Path
from typing import Any

import yaml
from aioesphomeapi import APIClient
from aioesphomeapi.model import BinarySensorState, SensorState, SwitchState, TextSensorState
from aiohttp import WSMsgType, web

ROOT = Path(__file__).resolve().parent
CONFIG = json.loads((ROOT / "config.json").read_text(encoding="utf-8-sig"))
PRODUCT = "CrowPanel ESP32 E-Paper HMI 4.2-inch Display"
HOST = os.getenv("CROWPANEL_HOST", CONFIG["host"])
PORT = int(os.getenv("CROWPANEL_PORT", str(CONFIG["api_port"])))
WEB_PORT = int(os.getenv("CROWPANEL_WEB_PORT", str(CONFIG["web_port"])))
EXPECTED_NAME = CONFIG["expected_name"]
PSK = (yaml.safe_load((ROOT / "device-secrets.yaml").read_text(encoding="utf-8-sig"))
       or {}).get("crowpanel_api_key", "")
if not PSK:
    raise RuntimeError("crowpanel_api_key is missing from device-secrets.yaml")

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
log = logging.getLogger("crowpanel-local")

state: dict[str, Any] = {
    "connected": False, "led": None, "button": None, "ip": "-", "wifi_signal": None,
    "device": EXPECTED_NAME, "product": PRODUCT, "hardware": "V1.2", "last_error": "Starting",
}
clients: set[web.WebSocketResponse] = set()
client: APIClient | None = None
led_key = 0
entity_ids: dict[int, str] = {}
command_lock = asyncio.Lock()
state_changed = asyncio.Event()

async def publish() -> None:
    message = json.dumps(state, ensure_ascii=False)
    dead = []
    for ws in clients:
        try:
            await ws.send_str(message)
        except Exception:
            dead.append(ws)
    for ws in dead:
        clients.discard(ws)

def on_state(item: Any) -> None:
    if getattr(item, "missing_state", False):
        return
    if isinstance(item, SwitchState) and item.key == led_key:
        state["led"] = bool(item.state)
    elif isinstance(item, BinarySensorState) and entity_ids.get(item.key) == CONFIG["button_object_id"]:
        state["button"] = bool(item.state)
    elif isinstance(item, SensorState) and entity_ids.get(item.key) == CONFIG["wifi_object_id"]:
        state["wifi_signal"] = round(item.state, 1) if math.isfinite(item.state) else None
    elif isinstance(item, TextSensorState) and entity_ids.get(item.key) == CONFIG["ip_object_id"]:
        state["ip"] = item.state
    state_changed.set()
    loop = asyncio.get_running_loop()
    loop.create_task(publish())

async def connect_loop() -> None:
    global client, led_key, entity_ids
    while True:
        c = APIClient(HOST, PORT, "", noise_psk=PSK, expected_name=EXPECTED_NAME)
        client = c
        stopped = asyncio.Event()
        async def on_stop(expected: bool) -> None:
            state.update(connected=False, led=None, button=None, wifi_signal=None, last_error="Device disconnected. Reconnecting...")
            state_changed.set()
            stopped.set()
            await publish()
        try:
            log.info("Connecting to %s:%s", HOST, PORT)
            await asyncio.wait_for(c.connect(login=True, on_stop=on_stop), 20)
            info, entities, _ = await c.device_info_and_list_entities()
            led = next((e for e in entities if getattr(e, "object_id", "") == CONFIG["switch_object_id"]), None)
            if led is None:
                raise RuntimeError("LED switch entity was not found")
            led_key = led.key
            entity_ids = {e.key: e.object_id for e in entities}
            state.update(connected=True, device=info.name or PRODUCT, last_error="")
            c.subscribe_states(on_state)
            log.info("Connected to %s; LED key=%s", info.name, led_key)
            await publish()
            await stopped.wait()
        except asyncio.CancelledError:
            await c.disconnect(force=True)
            raise
        except Exception as exc:
            state.update(connected=False, led=None, button=None, wifi_signal=None, last_error="Unable to connect to the device. Retrying...")
            await publish()
            log.warning("ESPHome connection stopped: %s", exc)
        finally:
            try:
                await c.disconnect(force=True)
            except Exception:
                pass
        await asyncio.sleep(3)

async def index(_: web.Request) -> web.Response:
    return web.FileResponse(ROOT / "index.html")

async def app_state(_: web.Request) -> web.Response:
    return web.json_response(state)

async def set_led(request: web.Request) -> web.Response:
    global client
    if request.headers.get("Origin") not in (None, "http://" + request.host):
        raise web.HTTPForbidden()
    try:
        data = await request.json()
    except (ValueError, TypeError):
        return web.json_response({"error": "A JSON switch state is required"}, status=400)
    if not isinstance(data, dict) or type(data.get("on")) is not bool:
        return web.json_response({"error": "on must be true or false"}, status=400)
    value = data["on"]
    if client is None or not state["connected"] or state["led"] is None:
        return web.json_response({"error": "The device is not connected"}, status=503)
    try:
        async with command_lock:
            if client is None or not state["connected"] or state["led"] is None:
                return web.json_response({"error": "The device is not connected"}, status=503)
            if state["led"] == value:
                return web.json_response({"ok": True, "on": state["led"]})
            state_changed.clear()
            client.switch_command(led_key, value)
            async with asyncio.timeout(5):
                while True:
                    await state_changed.wait()
                    state_changed.clear()
                    if not state["connected"]:
                        raise ConnectionError("Device connection lost")
                    if state["led"] == value:
                        break
            return web.json_response({"ok": True, "on": state["led"]})
    except Exception as exc:
        return web.json_response({"error": "No device confirmation received. Check the connection and current state."}, status=503)

async def websocket(request: web.Request) -> web.WebSocketResponse:
    if request.headers.get("Origin") not in (None, "http://" + request.host):
        raise web.HTTPForbidden()
    ws = web.WebSocketResponse(heartbeat=20)
    await ws.prepare(request)
    clients.add(ws)
    await ws.send_json(state)
    async for msg in ws:
        if msg.type == WSMsgType.ERROR:
            break
    clients.discard(ws)
    return ws

def make_app() -> web.Application:
    app = web.Application()
    app.router.add_get("/", index)
    app.router.add_get("/api/state", app_state)
    app.router.add_post("/api/led", set_led)
    app.router.add_get("/ws", websocket)
    return app

async def main() -> None:
    task = asyncio.create_task(connect_loop())
    runner = web.AppRunner(make_app())
    await runner.setup()
    site = web.TCPSite(runner, "127.0.0.1", WEB_PORT)
    await site.start()
    log.info("Local control page: http://127.0.0.1:%s", WEB_PORT)
    try:
        await asyncio.Event().wait()
    finally:
        task.cancel()
        await runner.cleanup()

if __name__ == "__main__":
    asyncio.run(main())
