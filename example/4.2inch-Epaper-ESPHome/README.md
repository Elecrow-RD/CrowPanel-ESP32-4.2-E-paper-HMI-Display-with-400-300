# 4.2inch-Epaper-ESPHome

产品：**CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2**。

请先阅读 [完整中文教程与二次开发说明](CrowPanel_电脑本地灯光控制_完整教程.md)。网页界面为英文。

## 全新产品

1. **电脑端：**安装 Python 3.14 x64；安装 Git for Windows 用于固件编译。
2. **电脑端：**运行 `py -3.14 prepare.py`，填写自己的 Wi-Fi，生成匹配的两端密钥。
3. **电脑端：**安装 `requirements-build.txt` 中依赖，编译 `firmware/4.2inch-Epaper-ESPHome.yaml`。
4. **电脑端 + 产品端：**USB 连接产品，在电脑执行烧录；记录产品实际 IP。
5. **电脑端：**运行 `pc/setup.bat`，修改 `pc/config.json` 中的设备地址，运行 `pc/doctor.py` 检查。
6. **电脑端：**双击 `pc/start-device.bat`，打开 `http://127.0.0.1:8099`。
7. **产品端 + 电脑端：**分别按实体中键、点击网页开关，确认 LED 与屏幕同步。

## 已烧好固件的产品，换一台电脑

无需再次编译或烧录。复制 `pc` 文件夹，在 `device-secrets.yaml` 填入该产品匹配的 API 密钥，在 `config.json` 填实际地址和固件设备名，重新运行 `setup.bat` 安装环境，再启动网页。

本教学工程设备名默认 `epaper-42`，mDNS 地址为 `epaper-42.local`。旧演示设备可能使用 `crowpanel-light`；这不是文件，具体说明见完整教程第 0 章。

不要分发自己的 secrets 文件、虚拟环境、Flash 备份或带密码的编译固件。共享源码时使用原始教学 ZIP。
