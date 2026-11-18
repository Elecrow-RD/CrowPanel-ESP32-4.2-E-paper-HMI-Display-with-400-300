"""Create per-user credentials without changing an already configured project."""
import base64
import getpass
import ipaddress
import json
from pathlib import Path
import re
import secrets

ROOT = Path(__file__).resolve().parent


def write_yaml(path, values):
    text = '# Private configuration: do not publish this file.\n'
    text += ''.join(f'{k}: {json.dumps(v, ensure_ascii=False)}\n' for k, v in values.items())
    with path.open('x', encoding='utf-8', newline='\n') as f:
        f.write(text)


def valid_host(value):
    try:
        return ipaddress.ip_address(value).version == 4
    except ValueError:
        return bool(re.fullmatch(r'[A-Za-z0-9](?:[A-Za-z0-9.-]{0,251}[A-Za-z0-9])?', value))


def main():
    targets = [ROOT/'firmware/secrets.yaml', ROOT/'pc/device-secrets.yaml']
    existing = [p for p in targets if p.exists()]
    if existing:
        print('Configuration already exists. Edit it manually; credentials were not changed.')
        for p in existing:
            print(p.relative_to(ROOT))
        return 1
    print('CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2')
    print('Project: 4.2inch-Epaper-ESPHome. Default device hostname: epaper-42.')
    ssid = input('2.4 GHz Wi-Fi SSID: ').strip()
    password = getpass.getpass('Wi-Fi password (hidden): ')
    if not ssid or len(ssid.encode('utf-8')) > 32:
        print('SSID must contain 1 to 32 UTF-8 bytes.')
        return 1
    if password and not 8 <= len(password.encode('utf-8')) <= 63:
        print('Use an 8-63 byte Wi-Fi passphrase, or empty for an open network.')
        return 1
    host = input('Device IP or hostname [epaper-42.local]: ').strip() or 'epaper-42.local'
    if not valid_host(host):
        print('Enter an IPv4 address or hostname, without http://, a port, or a path.')
        return 1
    key = base64.b64encode(secrets.token_bytes(32)).decode('ascii')
    values = {
        'crowpanel_wifi_ssid': ssid,
        'crowpanel_wifi_password': password,
        'crowpanel_api_key': key,
        'crowpanel_ota_password': secrets.token_urlsafe(24),
        'crowpanel_ap_password': secrets.token_urlsafe(16),
    }
    for p in targets:
        p.parent.mkdir(parents=True, exist_ok=True)
    write_yaml(targets[0], values)
    write_yaml(targets[1], {'crowpanel_api_key': key})
    config_file = ROOT/'pc/config.json'
    cfg = json.loads(config_file.read_text(encoding='utf-8-sig'))
    cfg['host'] = host
    config_file.write_text(json.dumps(cfg, indent=2)+'\n', encoding='utf-8')
    print('Created firmware/secrets.yaml and pc/device-secrets.yaml with matching API keys.')
    print('Updated pc/config.json. Keep private configuration and compiled firmware private.')
    print('Next: install ESPHome, build firmware, flash your device, then run pc/setup.bat.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
