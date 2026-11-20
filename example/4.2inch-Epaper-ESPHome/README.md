# 4.2inch-Epaper-ESPHome

Product: **CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2**.

Read first: [CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2: - Elecrow Wiki](https://www.elecrow.com/wiki/CrowPanel_4.2-Epaper-ESPHome.html)

## Setting Up a New Device

1. **Computer:** Install Python 3.14 x64 and Git for Windows to compile the firmware.
2. **Computer:** Run `py -3.14 prepare.py`, enter your Wi-Fi credentials, and generate the matching keys for both endpoints.
3. **Computer:** Install the dependencies listed in `requirements-build.txt` and compile `firmware/4.2inch-Epaper-ESPHome.yaml`.
4. **Computer + Device:** Connect the device to the computer via USB and flash it from the computer. Record the device’s actual IP address.
5. **Computer:** Run `pc/setup.bat`, update the device address in `pc/config.json`, and run `pc/doctor.py` to check the setup.
6. **Computer:** Double-click `pc/start-device.bat` and open `http://127.0.0.1:8099`.
7. **Device + Computer:** Press the physical center button and click the web switch. Confirm that the LED and display stay in sync.

## Using a Device with Firmware Already Flashed on Another Computer

There is no need to compile or flash the firmware again. Copy the `pc` folder, enter the API key for that device in `device-secrets.yaml`, and enter its actual IP address and firmware device name in `config.json`. Run `setup.bat` again to install the environment, then start the web page.

The default device name in this tutorial project is `epaper-42`, and its mDNS address is `epaper-42.local`. The older demo device may use `crowpanel-light`; this is a device name, not a file. See Chapter 0 of the full tutorial for details.

Do not distribute your own secrets file, virtual environment, Flash backup, or compiled firmware containing passwords. Use the original tutorial ZIP when sharing the source code.
