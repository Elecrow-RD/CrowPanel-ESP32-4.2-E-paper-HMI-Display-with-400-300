# CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2

**[Open the online tutorial](https://www.elecrow.com/wiki/CrowPanel_4.2-Epaper-ESPHome-VM.html) · [打开在线教程](https://www.elecrow.com/wiki/CrowPanel_4.2-Epaper-ESPHome-VM.html)**

## English · Start here

Build a light control panel using ESPHome and Home Assistant. Press the CrowPanel’s center button to turn an external LED on or off, or click the switch in your browser. The LED, the bulb icon and ON/OFF text on the e-paper display, and the dashboard switch all show the same state. Home Assistant runs in VMware Workstation on your Windows PC; no Raspberry Pi is needed.

**Before you begin:** Have a Windows x64 PC, the V1.2 display, a USB data cable, an external LED module, and connecting wires ready. The display needs 2.4 GHz Wi-Fi on the same local network as the PC. Internet access is needed for the initial setup.

If you downloaded a ZIP file, extract the whole package and keep its folders together. Open the **[online tutorial](https://www.elecrow.com/wiki/CrowPanel_4.2-Epaper-ESPHome-VM.html)** and start with Section 1. Each step tells you whether to work on the PC, in the virtual machine, or on the device.

| Sections | What you will do |
| --- | --- |
| 1–3 | Find the project files, prepare VMware, and wire the LED with the display powered off. |
| 4–6 | Install Home Assistant, set up ESPHome, and flash the display over USB using the supplied [firmware project](firmware/4.2inch-Epaper-ESPHome.yaml). |
| 7–9 | Add the display to Home Assistant, create the dashboard using [dashboard.yaml](dashboard.yaml), and test both the web switch and the center button. |
| 10–11 | After both controls work, use these sections to customize the design, back it up, or move it to another PC. |

## 中文 · 从这里开始

本教程使用 ESPHome 和 Home Assistant 搭建灯光控制面板。按下墨水屏中间按键，或点击浏览器中的开关，都能控制外接 LED。灯的亮灭、墨水屏上的灯泡图案和 ON/OFF 文字，以及网页开关的状态会同步更新。Home Assistant 运行在 Windows 电脑的 VMware Workstation 虚拟机中，无需树莓派。

**开始前请准备：** Windows x64 电脑、V1.2 墨水屏、支持数据传输的 USB 线、外接 LED 模块和连接线。墨水屏需要连接 2.4 GHz Wi-Fi，并与电脑处于同一局域网。首次搭建时需要访问互联网。

如果下载的是 ZIP 压缩包，请先完整解压，并保留目录结构。打开**[在线教程](https://www.elecrow.com/wiki/CrowPanel_4.2-Epaper-ESPHome-VM.html)**，从第 1 节开始。每一步都标明了是在电脑端、虚拟机端还是产品端操作。

| 章节 | 你将完成的操作 |
| --- | --- |
| 第 1–3 节 | 找到工程文件，准备 VMware，在墨水屏断电时接好 LED。 |
| 第 4–6 节 | 安装 Home Assistant，配置 ESPHome，使用随附的[固件工程](firmware/4.2inch-Epaper-ESPHome.yaml)通过 USB 烧录墨水屏。 |
| 第 7–9 节 | 将墨水屏添加到 Home Assistant，用 [dashboard.yaml](dashboard.yaml) 创建仪表盘，分别测试网页开关和中间按键。 |
| 第 10–11 节 | 两种控制方式都正常后，再按需查看如何修改设计、备份工程或迁移到另一台电脑。 |
