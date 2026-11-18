"""Read-only portable installation and encrypted device connection check."""
import asyncio
import base64
import json
import platform
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent


async def main():
    print('CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2')
    print('Python:', sys.version.split()[0], 'OS:', platform.platform())
    try:
        import yaml
        from aioesphomeapi import APIClient
        cfg = json.loads((ROOT/'config.json').read_text(encoding='utf-8-sig'))
        key = yaml.safe_load((ROOT/'device-secrets.yaml').read_text(encoding='utf-8-sig'))['crowpanel_api_key']
        if len(base64.b64decode(key, validate=True)) != 32:
            raise ValueError('API key must decode to 32 bytes')
        client = APIClient(cfg['host'], cfg['api_port'], '', noise_psk=key,
                           expected_name=cfg['expected_name'])
        try:
            await asyncio.wait_for(client.connect(login=True), 20)
            info, entities, _ = await client.device_info_and_list_entities()
            switches = [e for e in entities if type(e).__name__ == 'SwitchInfo']
            if not any(e.object_id == cfg['switch_object_id'] for e in switches):
                raise ValueError('Configured switch not found. Available: ' + ', '.join(e.object_id for e in switches))
            print('PASS: encrypted device connection, expected name, switch entity')
            print('Device:', info.name, 'MAC:', info.mac_address)
            print('Entity object IDs:', ', '.join(e.object_id for e in entities))
            return 0
        finally:
            await client.disconnect(force=True)
    except Exception as exc:
        print('FAIL:', type(exc).__name__, str(exc))
        print('Check config.json, device-secrets.yaml, power, Wi-Fi and TCP 6053.')
        return 1


if __name__ == '__main__':
    raise SystemExit(asyncio.run(main()))
