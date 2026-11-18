# CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2：电脑本地控制与二次开发教程

> 目标：不使用树莓派，不安装 Home Assistant，让 Windows 电脑网页直接控制墨水屏上的外接 LED，并与实体按键、灯泡图标和 ON/OFF 文字同步。
>
> 迁移方法：已有产品换电脑见第 12.11 节；全新产品从第 6 章开始。修改自己的设计见第 12.7～12.10 节。
>
> 配套源码：本教程所在的 `4.2inch-Epaper-ESPHome` 文件夹。请下载并解压完整源码包，不能只复制一个 YAML 或 HTML 文件。
>
> 文档日期：2026-11-18。适用硬件：**CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2，400 × 300 黑白墨水屏**。

## 目录

请先阅读第 0 章：操作位置、工程命名和 `.local` 主机名说明。

1. [完成后能做什么](#1-完成后能做什么)
2. [系统原理与设备分工](#2-系统原理与设备分工)
3. [硬件、网络与软件准备](#3-硬件网络与软件准备)
4. [接线与引脚选择](#4-接线与引脚选择)
5. [领取源码并认识文件](#5-领取源码并认识文件)
6. [安装 Python 并生成自己的配置](#6-安装-python-并生成自己的配置)
7. [在电脑上编译和首次烧录](#7-在电脑上编译和首次烧录)
8. [确认联网并找到设备 IP](#8-确认联网并找到设备-ip)
9. [安装并启动电脑网页](#9-安装并启动电脑网页)
10. [实际操作与验收](#10-实际操作与验收)
11. [关键代码逐段讲解](#11-关键代码逐段讲解)
12. [后续更新、迁移与扩展](#12-后续更新迁移与扩展)
13. [常见问题排查](#13-常见问题排查)
14. [授课与交付建议](#14-授课与交付建议)
15. [验证范围与参考资料](#15-验证范围与参考资料)

## 0. 使用本教程前：操作位置、命名和 `.local` 的含义

### 0.1 【电脑端 / 产品端】先分清在哪里操作

本教程中的“产品”统一指 **CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2**。

| 标记 | 实际在哪里操作 | 举例 |
| --- | --- | --- |
| 电脑端：编辑 | Windows 电脑上的项目目录 | 编辑 YAML、Python、HTML、JSON、PNG |
| 电脑端：命令 | Windows PowerShell / 启动脚本窗口 | 安装依赖、编译、烧录命令、日志命令 |
| 产品端 | 实体产品及 LED 接线 | 接 USB、按中键、按 BOOT/RESET、观察屏幕与 LED |
| 路由器端 | 路由器管理页面 | 查看设备 IP、设置 DHCP 地址保留 |

**所有源码都在电脑上修改。** 产品端不会出现一个让你编辑 YAML 的文件夹。设备代码修改后必须在电脑编译，再经 USB 或 OTA 写入产品才生效。网页样式修改只影响电脑页面，不会改变产品墨水屏。

### 0.2 【电脑端】工程名、产品名和网络名分别是什么

| 名称 | 本项目取值 | 定义位置 |
| --- | --- | --- |
| 产品名 | CrowPanel ESP32 E-Paper HMI 4.2-inch Display | 文档、网页标题、固件 `friendly_name` |
| 硬件版本 | V1.2 | 本教程适用硬件与驱动 |
| 工程名 | 4.2inch-Epaper-ESPHome | 根目录和 YAML 文件名 |
| 固件网络名 | epaper-42 | `firmware/4.2inch-Epaper-ESPHome.yaml` 中 `substitutions.name`，传给 `esphome.name` |
| mDNS 主机名 | epaper-42.local | 产品联网后由 ESPHome mDNS 广播 |
| 电脑网页地址 | http://127.0.0.1:8099 | 本机服务 `pc/config.json` 中 `web_port` |

工程目录和文件名可以包含点及大写字符；设备网络名应使用适合主机名规则的小写字母、数字和连字符，所以没有直接使用 `4.2inch-Epaper-ESPHome` 作为设备网络名。

### 0.3 【电脑端 / 产品端】`crowpanel-light.local` 文件在哪里？

**它不是文件，也不在任何目录中。** `crowpanel-light.local` 是此前演示固件使用的网络名称；此前 YAML 设置了 `name: crowpanel-light`，设备联网后便广播这个名称。新教学工程设置为 `epaper-42`，对应 `epaper-42.local`。

电脑用这个主机名查找产品的 IP，这个过程叫 mDNS。产品没通电、Wi-Fi 不通或网络不支持 mDNS 时，名称就可能无法解析。此时从串口日志或路由器查出实际 IP，填入 `pc/config.json` 的 `host` 即可，不需要寻找或创建一个 `.local` 文件。

读取此工程之前已烧录的产品时，以设备实际名称为准。`friendly_name` 是显示名称，不等于网络名。

## 1. 完成后能做什么

完成本教程后，你将得到一个独立运行的灯光面板：

- 按下墨水屏中间的物理滚动按键，LED 亮灭翻转。
- 墨水屏切换亮灯/灭灯图标，并显示 ON/OFF。
- 在电脑浏览器点击开关，LED 和墨水屏随之改变。
- 实体按键改变灯光后，电脑网页自动更新，不需要手动刷新。
- 网络断开时，实体按键仍能控制 LED；网页显示断线并禁用开关。
- 网络恢复后，电脑服务自动重连并重新读取设备状态。

本教程实现的是**同一局域网内的单设备控制**。默认网页只在运行服务的电脑上开放，不包含手机访问、互联网远程控制或多用户账户系统。

LED 通断很快，墨水屏完成一次物理刷新需要时间。正常现象是 LED 先改变，随后屏幕完成更新；不要将刷新等待误认为程序无响应。

## 2. 系统原理与设备分工

```text
电脑浏览器：http://127.0.0.1:8099
        │ HTTP 命令 / WebSocket 状态
        ▼
电脑 Python 服务：server.py
        │ ESPHome 加密 API，TCP 6053
        │ 局域网 Wi-Fi / 以太网
        ▼
CrowPanel ESP32-S3
        ├─ GPIO5：读取中间按键
        ├─ GPIO16：控制外接 LED
        └─ SPI：更新 400 × 300 墨水屏
```

| 部分 | 负责什么 | 是否一直需要 |
| --- | --- | --- |
| ESP32-S3 固件 | 按键、GPIO 输出、屏幕绘制、Wi-Fi、API | 设备运行时需要 |
| Python 编译环境 | 把 YAML 和显示驱动编译成固件 | 编译、修改固件时需要 |
| Python 网页服务 | 把网页操作转换成 ESPHome 命令，接收状态 | 使用电脑网页时需要 |
| 浏览器 | 显示页面、发送用户点击 | 使用网页时需要 |
| USB 数据线 | 初次烧录、串口日志，也可供电 | 首次烧录需要；日常可仅作供电 |
| 路由器或接入点 | 让电脑和 ESP32 互通 | 使用网页控制时需要 |

这里依然使用 **ESPHome 固件**，但 ESPHome 不要求一定配合 Home Assistant。电脑程序使用 `aioesphomeapi` 库，就能直接连接设备。

电脑关机后，网页控制停止，墨水屏的本地按键功能继续运行。USB 接在电脑上并不代表控制命令通过 USB 发送；本方案日常控制走网络。

### 三个地址不要混淆

| 地址/端口 | 含义 | 示例 |
| --- | --- | --- |
| `127.0.0.1:8099` | 这台电脑上的网页服务 | 浏览器打开 `http://127.0.0.1:8099` |
| 设备 IP 的 `6053` 端口 | 墨水屏 ESPHome API | Python 连接 `192.168.2.91:6053` |
| `COM44` 等串口号 | 电脑连接设备的 USB 串口 | 仅用于本次电脑识别到的串口 |

`192.168.2.91` 和 `COM44` 都是演示实例，**每位用户应填写自己的设备 IP 和串口号**。不要在浏览器里把设备的 6053 端口当作网页打开。

## 3. 硬件、网络与软件准备

### 3.1 【电脑端 + 产品端】硬件清单


| 物品 | 要求 |
| --- | --- |
| CrowPanel ESP32 E-Paper HMI 4.2-inch Display | 4.2 英寸、ESP32-S3、400 × 300，V1.2 版本 |
| Windows 电脑 | 教程命令面向 Windows 10/11 x64 |
| USB 数据线 | 能传输数据，不是只能充电的线 |
| LED | 优先使用普通小功率指示 LED |
| 限流电阻 | 330Ω，一只 |
| 杜邦线/连接线 | 能连接 GPIO16 和电源/地 |
| Wi-Fi | ESP32 可用的 2.4 GHz 网络 |

本例硬件识别结果为 8 MB Flash、8 MB PSRAM。当前功能不要求启用 PSRAM。

如果你使用 LED 灯条、功率灯或继电器模块，GPIO16 只能接合适的驱动输入，不能直接给大负载供电。必须按模块要求使用电源、驱动电路和共地连接。

### 3.2 【电脑端 / 路由器端】网络条件


1. 电脑和墨水屏处于可以互相访问的局域网。
2. 电脑可以接网线，墨水屏连接同一路由器的 2.4 GHz Wi-Fi。
3. 不使用会隔离终端的访客网络/AP 隔离设置。
4. 首次安装 Python 依赖和编译工具链需要互联网；完成后，本地控制不依赖互联网。

### 3.3 【电脑端】软件版本


| 软件 | 本教程使用的版本/方式 |
| --- | --- |
| Python | 3.14 x64，安装 Python Launcher，命令为 `py -3.14` |
| Git for Windows | 固件编译所需工具；安装后 `git --version` 可用 |
| ESPHome 编译器 | `2026.10.2`，见 `requirements-build.txt` |
| esptool | `5.4.0`，用于识别芯片、备份和烧录 |
| 电脑服务依赖 | 见 `pc/requirements.txt`，包括 aioesphomeapi、aiohttp、PyYAML |
| 浏览器 | Edge / Chrome 等现代浏览器 |

编译环境和电脑服务环境使用两个虚拟环境，避免不同依赖互相影响。`.venv` 文件夹不要直接复制到别人的电脑，应在目标电脑重新安装。

## 4. 接线与引脚选择

### 4.1 【电脑端核对资料 / 产品端识别接口】已核对的引脚表


| 功能 | GPIO | 说明 |
| --- | --- | --- |
| 中间按键 CONF | 5 | 按下为低电平 |
| 外接 LED | 16 | 扩展接口可用 GPIO；原理图 U5 第 18 脚 |
| 屏幕供电使能 | 7 | 高电平使能 |
| SPI MOSI | 11 | 图像数据 |
| SPI CLK | 12 | SPI 时钟 |
| 屏幕 CS | 45 | 片选 |
| 屏幕 DC | 46 | 命令/数据选择 |
| 屏幕 RESET | 47 | 复位 |
| 屏幕 BUSY | 48 | 刷新忙状态 |

GPIO 编号与连接器物理脚号不是同一回事。接线时看板上丝印和原理图，不能把“GPIO16”理解成“排针第 16 脚”。屏幕相关引脚已在板内连接，不需要再外接这些线。

### 4.2 【电脑端 + 产品端】推荐教学接法：普通 LED，低电平点亮


本源码默认 `inverted: true`，对应下列接法：

```text
板上 3.3V ── 330Ω 电阻 ── LED 正极（A）
                              LED 负极（K）── GPIO16
```

- GPIO16 输出 LOW：电流流过 LED，亮灯，逻辑状态为 ON。
- GPIO16 输出 HIGH：两端电位接近，灭灯，逻辑状态为 OFF。
- 接线前断开电源。不要省略限流电阻。
- LED 长脚通常为正极，但应结合器件标记确认。

如果采用“GPIO16 → 电阻 → LED 正极，LED 负极 → GND”的接法，则是**高电平点亮**，必须把 YAML 中 LED 的 `inverted: true` 改成 `inverted: false`，重新编译烧录。

### 4.3 【电脑端 + 产品端】使用带驱动的 LED 模块


根据模块手册接 VCC、GND、控制输入。GPIO16 接控制输入，模块和 CrowPanel 共地。输入应兼容 3.3V 逻辑，确认模块为高电平有效还是低电平有效后再选择 `inverted`。

不要照搬普通 LED 的接法给 5V/12V 灯条供电，也不要让外部高电压进入 GPIO。

## 5. 领取源码并认识文件

### 5.1 【电脑端】解压位置


建议解压到短、无空格、无中文的路径，例如：

```text
C:\4.2inch-Epaper-ESPHome
```

Windows 上部分编译工具对路径比较敏感。教程后面的 PowerShell 命令均以此路径为例。

### 5.2 【电脑端】完整文件结构


```text
4.2inch-Epaper-ESPHome/
├─ CrowPanel_电脑本地灯光控制_完整教程.md
├─ prepare.py                         # 生成用户私密配置
├─ requirements-build.txt             # ESPHome / esptool 编译依赖
├─ firmware/
│  ├─ 4.2inch-Epaper-ESPHome.yaml            # 完整设备配置
│  ├─ secrets.example.yaml            # 字段说明模板
│  ├─ secrets.yaml                    # 运行 prepare.py 后生成，不分发
│  ├─ assets/
│  │  ├─ light-on.png                 # 400×300 亮灯画面
│  │  └─ light-off.png                # 400×300 灭灯画面
│  └─ components/crowpanel_epaper/
│     ├─ __init__.py
│     ├─ display.py                   # ESPHome 配置接口
│     ├─ crowpanel_epaper.h
│     └─ crowpanel_epaper.cpp         # V1.2 版本屏幕驱动
└─ pc/
   ├─ server.py                       # 加密 API + HTTP + WebSocket
   ├─ doctor.py                       # 只读检查环境、密钥、设备名和实体
   ├─ index.html                      # 完整网页，不依赖外部 CDN
   ├─ requirements.txt                # 固定依赖版本
   ├─ setup.bat                       # 安装电脑服务环境
   ├─ start.bat                       # 启动服务
   ├─ start-device.bat                # 双击运行的便捷入口
   ├─ config.json                     # 设备地址、名称、端口和实体映射
   ├─ device-secrets.example.yaml     # API 密钥模板
   └─ device-secrets.yaml             # prepare.py 生成，不分发
```

### 5.3 【电脑端】哪些是公开源码，哪些是用户私密文件


可以分发：本教程、YAML 主配置、屏幕驱动、图片、Python/HTML、依赖清单、`.example.yaml` 模板。

不要分发：`firmware/secrets.yaml`、`pc/device-secrets.yaml`、自己的 Flash 备份、包含真实配置的编译输出。**编译后的固件也可能包含 Wi-Fi 密码和 API 密钥。** 

本教学包因此不附现有设备的 BIN 固件。

## 6. 安装 Python 并生成自己的配置

### 6.1 【电脑端】安装 Python


从 [Python 官方 Windows 下载页面](https://www.python.org/downloads/windows/) 安装 Python 3.14 x64，包含 Python Launcher。安装后重新打开 PowerShell，输入：

```powershell
py -3.14 --version
```

预期输出 `Python 3.14.x`。若 `py` 不存在，重新检查 Launcher 安装；若电脑已有多个 Python，不要凭 `python` 命令猜测正在使用哪个版本。

### 6.2 【电脑端】生成 Wi-Fi 和密钥文件


在 PowerShell 执行：

```powershell
cd C:\4.2inch-Epaper-ESPHome
py -3.14 prepare.py
```

按提示输入：

1. 2.4 GHz Wi-Fi 名称，区分大小写。
2. Wi-Fi 密码，输入时不显示字符，这是正常现象。
3. 设备 IP 或主机名。如果还不知道 IP，按回车使用 `epaper-42.local`，烧录后可以修改。

脚本会生成两个私密文件，并更新连接配置：

| 文件 | 内容 |
| --- | --- |
| `firmware/secrets.yaml` | Wi-Fi、API 密钥、OTA 密码、备用热点密码 |
| `pc/device-secrets.yaml` | 与设备一致的 API 密钥 |
| `pc/config.json` | 更新其中的设备地址；其余配置保留 |

脚本只需运行一次。如果任一私密文件已存在，它会退出而不覆盖，避免生成新密钥后电脑与现有固件失去匹配。

### 6.3 【电脑端】配置文件的关系


设备端示意：

```yaml
crowpanel_wifi_ssid: "用户自己的 Wi-Fi 名称"
crowpanel_wifi_password: "用户自己的 Wi-Fi 密码"
crowpanel_api_key: "由 prepare.py 自动生成"
crowpanel_ota_password: "由 prepare.py 自动生成"
crowpanel_ap_password: "由 prepare.py 自动生成"
```

电脑端示意：

```yaml
crowpanel_api_key: "必须与设备端完全一致"
```

这里的占位文字不能直接作为有效 API 密钥使用。API 密钥由 **32 个随机字节经过 Base64 编码**得到，不是任意取一个密码。

`prepare.py` 用下面的标准库代码生成它：

```python
import base64
import secrets

key = base64.b64encode(secrets.token_bytes(32)).decode("ascii")
```

API 密钥用于电脑与设备通信；OTA 密码用于无线更新；备用热点密码用于设备无法连接 Wi-Fi 时的设置热点。这三者用途不同。

## 7. 在电脑上编译和首次烧录

> 如果设备已由你预先烧好本项目固件，用户也拿到了与该设备匹配的 API 密钥，并且设备已联网，可以直接进入第 8、9 章。全新设备从本章开始。

### 7.1 【电脑端】建立独立编译环境

**操作位置：电脑 PowerShell。产品此时无需连接 USB。** 先安装 [Git for Windows](https://git-scm.com/downloads/win)，安装选项允许命令行使用 Git，然后重新打开 PowerShell 检查：

```powershell
git --version
```

编译项目建议放在短路径，如 `C:\4.2inch-Epaper-ESPHome`；预留约 10 GB 空间用于 Python、工具链、框架和构建缓存。本文的 Windows 全新构建使用了电脑 HTTP 代理下载海外依赖，网络慢时按第 13.6 节操作。


在项目根目录执行：

```powershell
cd C:\4.2inch-Epaper-ESPHome
py -3.14 -m venv .venv-build
.\.venv-build\Scripts\python.exe -m pip install -r requirements-build.txt
.\.venv-build\Scripts\python.exe -m esphome version
```

预期最后显示 ESPHome `2026.10.2`。本教程直接调用虚拟环境里的 Python，不需要执行 PowerShell 激活脚本，也就不需要调整 PowerShell 执行策略。

首次编译会额外下载 ESP-IDF、编译工具和相关依赖，耗时取决于网络和电脑。若依赖版本无法取得，应记录错误并向教程维护者核对版本，不要随意混用旧版驱动和新版组件接口。

### 7.2 【电脑端】校验配置


```powershell
.\.venv-build\Scripts\python.exe -m esphome config .\firmware\4.2inch-Epaper-ESPHome.yaml
```

预期出现配置有效的提示。若出现找不到 `crowpanel_epaper`、图片或 secret，先检查第 5 章的文件结构。

### 7.3 【电脑端】编译固件


```powershell
.\.venv-build\Scripts\python.exe -m esphome compile .\firmware\4.2inch-Epaper-ESPHome.yaml
```

成功标志通常包含 `Successfully compiled program`。记下日志中实际输出的 `firmware.factory.bin` 路径。

可以用 PowerShell 查找：

```powershell
Get-ChildItem .\firmware -Recurse -Force -Filter firmware.factory.bin |
  Select-Object FullName, LastWriteTime
```

输出路径受构建后端影响。本次 Windows ESP-IDF 构建已验证的路径为：

```text
firmware/.esphome/build/epaper-42/build/firmware.factory.bin
```

不同构建后端或版本可能使用其他子目录。**后面烧录应使用本次编译日志列出的文件，不要选过期文件。**

### 7.4 【电脑端 + 产品端】确认 USB 串口与芯片


连接 USB 数据线，在 Windows 设备管理器 → 端口（COM 和 LPT）查看设备串口。

如果没有出现新串口，检查数据线和 CH340/CH34x 串口驱动。关掉占用该串口的 Arduino 串口监视器、其他烧录工具或终端。

下面以 `COM44` 为例，请替换成自己的端口：

```powershell
.\.venv-build\Scripts\python.exe -m esptool --port COM44 flash-id
```

本硬件预期识别为 ESP32-S3，Flash 为 8 MB。

### 7.5 【电脑端 + 产品端】可选：保存出厂或已有固件


如果想保留现有程序，先备份整片 Flash：

```powershell
New-Item -ItemType Directory -Force .\backup
.\.venv-build\Scripts\python.exe -m esptool --port COM44 --baud 921600 read-flash 0 0x800000 .\backup\original.bin
```

`0x800000` 对应 8 MB，只适用于确认容量后的本例。备份文件可能包含原设备凭据，自己保存即可。

### 7.6 【电脑端 + 产品端】烧录 factory 固件


将 `$factory` 设置为第 7.3 节查到的实际路径：

```powershell
$factory = '.\firmware\.esphome\build\epaper-42\build\firmware.factory.bin'
Test-Path $factory
.\.venv-build\Scripts\python.exe -m esptool --port COM44 --baud 921600 write-flash 0x0 $factory
```

`Test-Path` 应输出 `True`。本命令的 `0x0` 适用于 ESP32-S3 的**合并 factory 固件**，不要把单独的 OTA 应用固件按此地址写入。

成功时会出现写入完成、哈希校验通过和复位提示。若高速烧录不稳定，把 `921600` 降为 `460800` 或 `115200`。

如果一直停在 `Connecting...`，先检查端口、USB 线和驱动。仍不成功时，根据板上标记按住 BOOT，按一下 RESET 后松开 BOOT，再尝试烧录。

## 8. 确认联网并找到设备 IP

### 8.1 【电脑端 + 产品端】查看串口日志


烧录工具退出后执行：

```powershell
.\.venv-build\Scripts\python.exe -m esphome logs .\firmware\4.2inch-Epaper-ESPHome.yaml --device COM44
```

串口日志波特率由配置中的 `logger` 决定，本项目使用 115200。必要时按一次 RESET 捕获完整启动日志。

重点检查：

```text
Connected
IP Address: 192.168.x.xxx
Rendering LED OFF
Refresh ... completed ...
```

具体日志格式可能略有不同。应能看到 Wi-Fi 获得 IP、屏幕初始化和状态绘制。记下设备实际 IP，然后按 Ctrl+C 退出日志。

### 8.2 【电脑端 / 路由器端】其他找 IP 的方式


- 在路由器的 DHCP/客户端列表中查找 `epaper-42`。
- 如果电脑支持 mDNS，可以使用 `epaper-42.local`；Windows 某些网络下解析不稳定，教学时优先填写实际 IP。
- 设备无法联网时检查 `firmware/secrets.yaml` 中的 SSID/密码，修改后重新编译烧录。

### 8.3 【电脑端 + 产品端】判断本地功能是否已完成


此时还没运行电脑网页，也应该能完成：

1. 启动后显示灭灯画面和 OFF。
2. 按中间键，LED 点亮，显示 ON。
3. 再按一次，LED 熄灭，显示 OFF。

如果这一步不成功，先排查硬件和固件，不要继续调整电脑网页。页面无法修复错误的 LED 接线或屏幕驱动。

## 9. 安装并启动电脑网页

### 9.1 【电脑端】安装电脑端依赖


双击：

```text
pc/setup.bat
```

它会在 `pc/.venv` 建立独立环境，并安装 `pc/requirements.txt` 中固定的依赖版本。成功后关闭安装窗口即可。网络正常时通常比首次固件编译快。

如果脚本报错，可在项目根目录逐条执行以定位问题：

```powershell
py -3.14 -m venv .\pc\.venv
.\pc\.venv\Scripts\python.exe -m pip install -r .\pc\requirements.txt
```

### 9.2 【电脑端】填写设备连接配置

**操作位置：电脑。修改文件：`pc/config.json`。生效方式：保存后重启电脑服务，不需要烧录产品。**

配置示例（IP 必须换成自己的）：

```json
{
  "host": "192.168.2.91",
  "api_port": 6053,
  "expected_name": "epaper-42",
  "web_port": 8099,
  "switch_object_id": "led",
  "button_object_id": "center_button",
  "wifi_object_id": "wifi_signal",
  "ip_object_id": "ip_address"
}
```

`host` 只填写 IP 或主机名，不写 `http://` 或端口。`expected_name` 必须与已烧录固件中的 `esphome.name` 相同。新用户使用配套固件时为 `epaper-42`；如果连接此前已经烧录的演示设备，其名称可能还是 `crowpanel-light`，应按实际名称填写，不需要为了改显示名称而重刷原设备。

在工程根目录执行只读连接检查：

```powershell
.\pc\.venv\Scripts\python.exe .\pc\doctor.py
```

出现 `PASS: encrypted device connection, expected name, switch entity` 才表示加密密钥、设备名和开关实体都已验证。该工具不会改变 LED。

电脑服务也支持环境变量临时覆盖 `CROWPANEL_HOST`、`CROWPANEL_PORT` 和 `CROWPANEL_WEB_PORT`。普通用户优先使用 `config.json`，不要一边改 JSON 一边保留相反的环境变量。

### 9.3 【电脑端】启动网页服务


双击 `pc/start-device.bat`，保持黑色命令窗口打开。

预期日志包含：

```text
Local control page: http://127.0.0.1:8099
Connected to epaper-42
```

浏览器打开：

```text
http://127.0.0.1:8099
```

不要直接双击 `index.html`。它需要 Python 服务提供 `/api/led` 和 `/ws`，单独打开 HTML 文件不能控制设备。

### 9.4 【电脑端】正常页面应显示什么


| 页面项目 | 正常表现 |
| --- | --- |
| 连接状态 | Connected |
| LED 状态 | ON 或 OFF，来自设备回报 |
| 开关按钮 | Turn LED ON / Turn LED OFF |
| 灯泡图标 | 随 ON/OFF 改变 |
| 设备 IP | 实际局域网 IP |
| Wi-Fi 信号 | 例如 -55 dBm，负值是正常的 |
| 实体按键 | 按下时显示 Pressed，松开后显示 Released |

网页所有标题、按钮、状态与错误提示均使用英文。按键状态可能只短暂显示，属于按键按住时间短的正常现象。

服务仅监听 `127.0.0.1`，因此**只能在这台电脑打开网页**，不需要为浏览器访问开放 Windows 入站防火墙端口。电脑仍需能主动连接设备的 TCP 6053 端口。

## 10. 实际操作与验收

### 10.1 【电脑端 + 产品端】网页控制


1. 确认页面显示“Connected”。
2. 点击“Turn LED ON”。
3. 观察外接 LED 点亮。
4. 墨水屏显示亮灯图标和 ON。
5. 网页在收到设备确认后显示 ON。
6. 点击“Turn LED OFF”，确认三个状态都变为 OFF。

### 10.2 【电脑端 + 产品端】实体按键控制


1. 保持网页打开。
2. 按一下中间按键。
3. LED 翻转，墨水屏完成对应刷新。
4. 电脑网页自动变为相同的 ON/OFF。

一次按下应只翻转一次。按键的 30ms 去抖用于滤除机械触点抖动。

### 10.3 【电脑端 + 产品端】独立运行验证


如果现场还有以前的树莓派服务器，可以正常关机后再重复上述测试。这能直观证明命令直接从电脑发往 ESP32。

不要强行拔掉正在写数据的树莓派电源。本电脑方案本身无需修改、停止或访问原 HA 配置。

### 10.4 【电脑端 + 产品端】断线与恢复验证


在安全方便的情况下短暂断开墨水屏电源，再重新上电：

- 检测到连接断开后，页面显示 Disconnected，按钮禁用，LED 状态显示为未知。
- 断线检测会受 TCP 超时影响，未必在拔电瞬间出现。
- 重新上电后 LED 默认 OFF。
- 设备重连 Wi-Fi 后，电脑服务自动连接并读取最新状态。

快速连续操作时，墨水屏可能先完成正在刷新的旧帧，再显示最新状态。这是刷新合并机制，不代表 LED 忽略了命令。

### 10.5 【电脑端 + 产品端】验收表


| 项目 | 合格标准 | 结果 |
| --- | --- | --- |
| 首次上电 | 正常启动、LED 默认 OFF | □ |
| 按键开灯 | LED 亮，屏幕 ON，网页 ON | □ |
| 按键关灯 | LED 灭，屏幕 OFF，网页 OFF | □ |
| 网页开灯 | LED 亮，屏幕 ON，网页 ON | □ |
| 网页关灯 | LED 灭，屏幕 OFF，网页 OFF | □ |
| 电脑服务关闭 | 本地按键仍然可用 | □ |
| 重新启动服务 | 自动读取当前灯光状态 | □ |
| 设备断线恢复 | 按钮恢复可用，状态正确 | □ |

日常结束时，在服务窗口按 Ctrl+C 或关闭窗口。下次直接双击 `start-device.bat`，无需重复安装和烧录。

## 11. 关键代码逐段讲解

**本章所有代码编辑均在电脑完成。** YAML/C++ 的修改会影响产品，但只有重新编译并烧录后才在产品生效；Python 修改需要重启电脑服务，HTML 修改需要刷新电脑浏览器。

完整代码在源码包中，本章只抽取关键部分讲解。**不要把以下片段当成完整 YAML 单独烧录。**

### 11.1 【电脑端】固件的唯一灯光状态


```yaml
switch:
  - platform: gpio
    pin:
      number: GPIO16
      inverted: true
    id: led_load
    name: LED
    restore_mode: ALWAYS_OFF
    on_turn_on:
      - component.update: epaper
    on_turn_off:
      - component.update: epaper
```

`led_load` 是同一个逻辑开关：

- GPIO 输出使用它。
- 按键翻转它。
- 网络 API 控制它。
- 屏幕读取它。

不要分别创建“网页状态”“按键状态”“LED 状态”三个互不关联的灯光变量，否则容易出现网页显示 ON 但实际输出 OFF。

`ALWAYS_OFF` 表示重启后关闭输出。它并不保证上电前的 GPIO 电气状态；功率驱动的安全初始状态还应由硬件保证。

### 11.2 【电脑端】按键逻辑


```yaml
binary_sensor:
  - platform: gpio
    id: center_key
    name: Center Button
    pin:
      number: GPIO5
      mode:
        input: true
        pullup: true
      inverted: true
    filters:
      - delayed_on: 30ms
      - delayed_off: 30ms
    on_press:
      - switch.toggle: led_load
```

这里 `inverted: true` 表示低电平是“按下”。它与 LED 上的 `inverted` 分别配置，各自独立。

本教程只使用滚动按键的**中间按压**，不使用向上/向下滚动动作。

### 11.3 【电脑端】屏幕更新


```yaml
external_components:
  - source:
      type: local
      path: components
    components: [crowpanel_epaper]
```

上面的 `components` 相对于 `4.2inch-Epaper-ESPHome.yaml` 所在目录，所以必须保留 `firmware/components`。

画面选择逻辑：

```cpp
it.image(0, 0,
  id(led_load).state ? id(light_on_image) : id(light_off_image));
```

`light-on.png` 和 `light-off.png` 都是 400 × 300 黑白图片。状态变化后调用 `component.update: epaper`，驱动将对应图像写入屏幕。

V1.2 版本的屏幕驱动按厂家示例移植，使用 GC 刷新波形。它把复位、发送、等待刷新拆成状态机，避免用长时间忙等堵住按键和网络处理。BUSY 超时会输出错误日志。

不要仅因尺寸为 4.2 英寸就替换成任意 `waveshare_epaper` 型号；不同屏幕版本的指令和波形可能不同。

### 11.4 【电脑端】电脑如何直接连接设备


`pc/server.py` 中的关键逻辑：

```python
c = APIClient(
    HOST,
    PORT,
    "",
    noise_psk=PSK,
    expected_name=CONFIG["expected_name"],
)
await c.connect(login=True, on_stop=on_stop)
info, entities, _ = await c.device_info_and_list_entities()
c.subscribe_states(on_state)
```

- `HOST`：设备地址。
- `PORT`：默认 6053。
- `PSK`：从 `device-secrets.yaml` 读取的 API 密钥。
- `expected_name`：从 config.json 读取，确认连接的是配置指定的设备。
- `subscribe_states`：订阅设备状态，所以按键操作能主动推送到电脑。

程序按实体 `object_id` 查找 `led`、`center_button`、`wifi_signal`、`ip_address`，不依赖本次设备的数字实体 key。

发送开关命令：

```python
client.switch_command(led_key, value)
```

`value` 是 `True` 或 `False`。收到设备回报后才确认命令；仅把消息写进网络连接不等于 LED 已经执行。

### 11.5 【电脑端】网页如何与 Python 通信


| 接口 | 方法 | 用途 |
| --- | --- | --- |
| `/` | GET | 返回 `index.html` |
| `/api/state` | GET | 获取当前连接与设备状态 |
| `/api/led` | POST | 提交 `{"on": true}` 或 `{"on": false}` |
| `/ws` | WebSocket | 实时推送 LED、按键、IP 和信号变化 |

网页请求示意：

```javascript
await fetch('/api/led', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({ on: true })
});
```

网页中的 ON/OFF 根据 WebSocket 回报更新，不是点击时立即假设成功。等待确认期间按钮会暂时禁用。

物理 LED 是否真的有电流仍需要观察或测量；API 回报证明固件状态，不能替代硬件测量。

## 12. 后续更新、迁移与扩展

### 12.1 【电脑端】只改网页


修改 `pc/index.html`，刷新浏览器即可，例如改标题、颜色或布局。浏览器缓存时使用 Ctrl+F5。

修改 `pc/server.py` 后，应关闭服务再重新启动。

### 12.2 【电脑端】修改屏幕图案或固件逻辑


修改 `firmware/assets` 图片、YAML 或显示驱动后，需要重新编译并更新设备，刷新浏览器不会改变墨水屏固件。

设备已联网且 OTA 凭据一致时，可使用：

```powershell
.\.venv-build\Scripts\python.exe -m esphome run .\firmware\4.2inch-Epaper-ESPHome.yaml --device 192.168.2.91
```

替换为实际 IP。此命令会编译、上传并进入日志，完成后 Ctrl+C 退出。OTA 不可用时仍可通过 USB 恢复。

### 12.3 【电脑端】修改 IP


设备 IP 由路由器 DHCP 分配，重启或更换路由器后可能变化。优先在路由器为设备 MAC 设置地址保留，而不是随意硬编码一个可能冲突的静态地址。

IP 变化后修改 `pc/config.json` 中的 `host` 并重启服务，不需要因此重新烧录。

### 12.4 【电脑端】更换电脑


1. 复制源码包的 `pc` 目录。
2. 私下复制与设备匹配的 `device-secrets.yaml`。
3. 在新电脑安装 Python，运行 `setup.bat`。
4. 配置设备地址并启动。

不要重新随机生成 API 密钥后只替换电脑端，否则它与设备固件不匹配。若确实要换密钥，应同时更新设备端并重新烧录或通过相应机制更新。

### 12.5 【电脑端】更改设备名称


本教程按单台设备设计，建议初学者保留 `epaper-42`。如果改 YAML 中的 `name`，还要同步修改 `pc/config.json` 的 `expected_name`，以及使用的 `.local` 主机名。更改实体名称时也要检查 `object_id` 是否随之变化。

### 12.6 【电脑端】手机或其他电脑访问


当前版本只监听电脑本机，因此手机访问 `http://电脑IP:8099` 不会成功。这是预期行为。

如果后续要扩展局域网多人访问，需要另行设计监听地址、防火墙范围和访问认证。不能只把地址改为 `0.0.0.0` 就当作完成了安全的远程控制产品。不要把本例直接做公网端口转发。

### 12.7 【电脑端】按自己的设计修改：先查这张表

| 想修改什么 | 在电脑上修改的文件/字段 | 电脑服务需要重启？ | 产品需要重新烧录？ | 产品端验收 |
| --- | --- | --- | --- | --- |
| 网页标题、英文提示、颜色、布局 | `pc/index.html` 的 HTML/CSS/JS | 不需要，Ctrl+F5 刷新 | 不需要 | 网页点击仍能控制产品 |
| 网页端口 | `pc/config.json` 的 `web_port` | 需要 | 不需要 | 新 URL 能读取当前状态 |
| 产品 IP 改变 | `pc/config.json` 的 `host` | 需要 | 不需要 | Connected，状态同步 |
| 产品网络名 | YAML 的 `substitutions.name`；JSON 的 `expected_name` 和需要时的 `host` | 需要 | 需要 | 日志显示新名称，doctor 通过 |
| 网页显示的设备名以外的产品标题 | `pc/index.html` | 刷新即可 | 不需要 | 页面标题正确 |
| 产品固件友好名称 | YAML 的 `esphome.friendly_name` | 通常不需要 | 需要 | 固件信息更新 |
| LED 输出脚 | YAML 的 `substitutions.led_pin` | 不需要 | 需要 | 检查新接线、本地和网页控制 |
| LED 高/低电平点亮 | YAML 的 LED `pin.inverted` | 不需要 | 需要 | ON 亮、OFF 灭 |
| 中键去抖或动作 | YAML 的 GPIO5 `filters` / `on_press` | 不需要 | 需要 | 一次按下只执行一次动作 |
| 产品显示的两张图片 | `firmware/assets/light-on.png`、`light-off.png` | 不需要 | 需要 | ON/OFF 图案无裁切、方向正确 |
| 根据其他状态显示文字 | YAML 的 display lambda、必要的 font 配置 | 不需要 | 需要 | 屏幕布局正确 |
| Wi-Fi 名称/密码 | `firmware/secrets.yaml` | 产品 IP 变化时更新 JSON 并重启 | 需要 | 重新联网、doctor 通过 |
| API 密钥 | 固件 secrets 和电脑 device-secrets 两端同时改 | 需要 | 需要 | 加密连接成功 |
| 修改实体名称/ID | YAML 与 `pc/config.json` 的 object_id 映射 | 需要 | 需要 | doctor 找到正确实体 |
| 增加第二路输出/传感器 | YAML、server.py 的状态/接口、index.html 的控件三处 | 需要 | 需要 | 两路独立且状态不串线 |

### 12.8 【电脑端编辑 → 电脑端编译 → 产品端验证】例一：改成自己的屏幕图片

1. **电脑端：**复制整个工程作为设计备份。用绘图软件制作两张 400×300 黑白 PNG，保持 `light-on.png`、`light-off.png` 文件名并覆盖 `firmware/assets/` 中相应文件。字体可直接画进图片，不必在固件中增加字体库。
2. **电脑端：**运行 `esphome config` 和 `esphome compile`，命令同第 7 章。编译成功后，使用 USB factory 烧录或第 12.2 节的 OTA 命令上传。
3. **产品端：**重启后观察 OFF 图案，再按中键观察 ON 图案；确保字没有被裁掉、灯泡与 LED 状态一致。
4. **电脑端网页：**点击开关验证新固件仍能回传状态。图片只改了产品画面，网页图标不会自动换图；若也要改网页，单独编辑 `pc/index.html` 的 SVG/CSS。

### 12.9 【电脑端编辑 → 产品端接线】例二：换成高电平有效 LED 模块

1. **产品端：**断电，确认模块输入兼容 3.3V，按模块手册接电源、地和 GPIO16。
2. **电脑端：**在 YAML 的 LED 开关中将 `inverted: true` 改为 `inverted: false`。不要改 GPIO5 按键的 inverted，因为按键极性没有变化。
3. **电脑端：**编译后 USB 或 OTA 上传。
4. **产品端 + 电脑端：**分别按中键和点网页，检查 ON 亮、OFF 灭。不要交换 ON/OFF 图片来补偿错误输出极性。

### 12.10 【电脑端】例三：新增一路输出需要改哪些代码

当前示例只支持一路 LED，不能仅在网页多复制一个按钮就控制第二路。

1. **电脑端：**核对原理图，选择未占用且适合输出的 GPIO，在 YAML `switch` 中增加第二个 GPIO 开关，使用不同 `id` 和名称。
2. **电脑端：**编译上传后运行 `pc/doctor.py`，查看实际实体 `object_id`。不要假定名称空格和大小写如何转换。
3. **电脑端：**在 `server.py` 增加第二个 switch key、状态字段、命令目标与回报确认。原有 `/api/led` 只接受一路布尔值，需设计区分两路的接口或参数。
4. **电脑端：**在 `index.html` 增加第二个控件，并映射到新接口和新状态字段。
5. **电脑端：**根据需求改 display lambda，让产品画面显示两路状态；仍需编译上传到产品。
6. **产品端 + 电脑端：**逐路测试，确认操作一路不会误切另一路，并验证断线恢复。

这一扩展是开发路线说明，本包没有预置或验证第二路固件。先保留原来可运行的一路版本，再逐步增加功能。

### 12.11 【电脑端】把工程交给另一位客户：分两种情况

**客户使用同一台已烧好固件的产品：**

1. 在新电脑安装 Python 3.14 x64，解压源码包。
2. 单独交付该产品匹配的 API 密钥。**电脑端：**把 `pc/device-secrets.example.yaml` 复制为 `pc/device-secrets.yaml`，将里面的占位值替换为实际 API 密钥。只使用网页时无需给 Wi-Fi 密码或安装编译器。
3. 修改 `pc/config.json` 的 host、expected_name，运行 setup.bat 和 doctor.py。
4. 启动 start-device.bat，按第 10 章验收。无需重刷产品，也不要为此重新运行 prepare.py 生成不匹配的密钥。

**客户拿到全新产品，使用自己的网络：**

1. 按第 6 章运行 prepare.py，填写自己的 Wi-Fi，生成自己的密钥。
2. 按第 7 章在电脑编译并写入产品。
3. 按第 8、9 章填写实际 IP 并安装启动电脑服务。
4. 按第 10 章完成实机验收。

两种情况都不复制 `.venv`、旧的 `.esphome` 构建目录或教师带密码的 BIN。客户电脑须能下载指定依赖，产品和电脑须能在局域网互通。无需 G 盘、原开发目录、树莓派地址或教师的个人账号。

## 13. 常见问题排查

### 13.1 【电脑端】页面打不开


按顺序检查：

1. 是否启动 `pc/start-device.bat`，服务窗口是否仍然存在。
2. URL 是否为 `http://127.0.0.1:8099`，不是设备 IP，也不是 HTTPS。
3. 服务窗口是否提示缺少依赖、找不到密钥文件或端口被占用。
4. 是否直接双击 HTML，而没有启动 Python。

PowerShell 可查询端口：

```powershell
Get-NetTCPConnection -LocalPort 8099 -State Listen
```

若已经有服务正常运行，不要重复启动。要换网页端口，可在 `start-device.bat` 中加 `set "CROWPANEL_WEB_PORT=8100"`，然后访问相应的新端口。

### 13.2 【电脑端】页面能打开，但一直未连接


```powershell
Test-NetConnection 192.168.2.91 -Port 6053
```

替换为实际 IP。`TcpTestSucceeded: True` 只证明端口可达，不代表加密认证成功。

| 现象 | 优先检查 |
| --- | --- |
| TCP 不通 | 设备电源、IP、Wi-Fi、访客网络隔离、路由 |
| TCP 通但认证失败 | 两端 API 密钥是否完全相同 |
| 设备名称不匹配 | YAML 名称与 `pc/config.json` 的 `expected_name` |
| 找不到 LED 实体 | 是否烧录了本教程固件，实体名称是否被修改 |
| `.local` 解析失败 | 用路由器或串口查到的实际 IP 替代 |

电脑和屏幕可以一台有线、一台无线，关键是网络互通，而不是连接方式相同。

### 13.3 【产品端检查接线 / 电脑端修改固件】ON 灭灯、OFF 亮灯


这是输出极性或接线方式不匹配。查看第 4 章，确认负载为低电平有效还是高电平有效，再修改 LED 的 `inverted`，重新编译烧录。

不要只把网页上的 ON/OFF 文案交换来掩盖错误，因为那会破坏实际状态的一致性。

### 13.4 【产品端】LED 不亮，但屏幕会变化


检查 GPIO16 是否接对、LED 正负极、限流电阻、模块电源与共地。板上 GPIO16 丝印与排针第 16 脚不是同一个概念。

### 13.5 【产品端检查 / 电脑端读取日志】屏幕白屏、乱码或 BUSY 超时


1. 确认硬件是V1.2 版本。
2. 确认整个 `components/crowpanel_epaper` 文件夹已复制。
3. 确认 GPIO7 电源使能及 SPI 引脚没有被改动或占用。
4. 检查 USB 供电与屏幕排线。
5. 查看串口错误，不要连续尝试不匹配的刷新波形。

### 13.6 【电脑端】首次编译下载慢或失败


先区分 pip 安装失败，还是编译时下载 GitHub/ESP-IDF 工具链失败。

如果电脑代理提供 HTTP 端口，可在**同一个 PowerShell 窗口**设置代理后重试：

```powershell
# 10809 是示例，请填写自己代理软件的 HTTP 或 mixed 端口。
$env:HTTP_PROXY = 'http://127.0.0.1:10809'
$env:HTTPS_PROXY = 'http://127.0.0.1:10809'
.\.venv-build\Scripts\python.exe -m esphome compile .\firmware\4.2inch-Epaper-ESPHome.yaml
```

代理软件要保持运行。pip 镜像只能帮助 pip 下载，不一定解决 GitHub 编译依赖下载。

网络恢复后重新编译，通常会复用已完成的缓存。不要因为下载慢就重新刷电脑或删除整套工程。

若清理该窗口的临时代理设置：

```powershell
Remove-Item Env:HTTP_PROXY, Env:HTTPS_PROXY -ErrorAction SilentlyContinue
```

日常电脑网页控制不需要这两个代理变量。

### 13.7 【电脑端】Python、编码或依赖报错


- 使用 `py -3.14` 确认版本，不要混用其他虚拟环境。
- YAML、Python、HTML 和依赖清单保存为 UTF-8。
- 不要把 `secrets.yaml` 保存成 `secrets.yaml.txt`；在资源管理器开启“文件扩展名”。
- 不要通过 PowerShell 5 的默认 `>` 随意重建依赖清单，它可能生成 UTF-16；优先使用包内文件。
- 安装失败时保留完整错误消息，尤其是包名、Python 版本和最先出现的错误。

### 13.8 【电脑端】Wi-Fi 与备用热点


ESP32-S3 使用 2.4 GHz Wi-Fi。SSID 和密码区分大小写，避免连接只允许特定认证方式的企业网络。

连接家庭 Wi-Fi 失败时，固件可能启动 `Epaper 4.2 Setup` 备用热点；密码在自己的 `firmware/secrets.yaml` 中。初学者建议先核对配置并重新烧录，使源码与设备配置保持一致。

### 13.9 【电脑端】网页按钮短暂等待或报确认超时


服务器等待设备回报最多约 5 秒。若超时，请看页面当前状态和服务窗口日志，排查无线信号、设备重启或其他控制端同时操作。不要只依据浏览器请求发出就认定开关已经执行。

## 14. 授课与交付建议

### 14.1 【电脑端】教师应给用户什么


分发本 `4.2inch-Epaper-ESPHome` 完整源码包，包括本教程、固件源文件、电脑端源文件和配置生成脚本。建议给示范用户预先准备普通 LED、电阻和标好 GPIO16 的接线图。

教学时让每位用户生成自己的配置。不要把教师正在使用的 Wi-Fi 密码、API 密钥、原设备 Flash 备份或编译 BIN 当作通用示例发布。

### 14.2 【电脑端】推荐演示顺序


| 阶段 | 演示内容 | 继续下一阶段前要确认 |
| --- | --- | --- |
| 认识系统 | 浏览器、电脑服务、ESP32 的分工 | 用户能区分串口、设备 IP、网页地址 |
| 硬件接线 | GPIO16、限流电阻、LED 极性 | 接线与 `inverted` 一致 |
| 生成配置 | 运行 prepare.py | 设备端与电脑端密钥一致 |
| 编译烧录 | Validate → Compile → Flash | 哈希校验通过 |
| 本地控制 | 只按中间键 | LED 和屏幕同步 |
| 网络控制 | 电脑启动服务、打开网页 | 点击开关能控制实物 |
| 状态回传 | 按实体键观察网页 | 无需刷新网页即可同步 |
| 故障练习 | 改错 IP、停止服务后恢复 | 会分层定位问题 |

### 14.3 【电脑端】用户求助时应收集什么


- 产品完整型号和 V1.2 硬件版本信息。
- LED 接线照片及负载类型。
- Windows、Python、ESPHome 版本。
- 实际串口号、设备 IP。
- 编译错误或服务窗口错误的关键部分。
- 是本地按键失败，还是仅网页失败。

不需要用户把 Wi-Fi 密码、API 密钥公开贴进群聊。

## 15. 验证范围与参考资料

### 15.1 【电脑端】已完成的验证


本项目已在实际 CrowPanel 设备上验证：

- ESPHome 2026.10.2 固件编译成功。
- 通过 Windows 电脑 COM44 烧录 factory 固件并完成写入哈希校验。
- 中间按键、GPIO16 低电平有效 LED、墨水屏 ON/OFF 图案正常。
- Windows 10 x64 + Python 3.14 电脑服务直接连接 ESPHome API。
- 浏览器实际 ON/OFF 操作、设备状态回报、网页更新正常。
- 断开 API 连接后自动重连并重新读取状态。
- 用户已确认电脑网页和实体按键都同步正常。

**本轮新增迁移验证：**已在 Windows 上建立全新电脑服务虚拟环境，重新安装依赖，将源码复制到另一目录，并从非项目工作目录启动。加密连接、英文页面、网页实际开关及窄屏布局均通过检查。配置生成脚本已验证两端密钥匹配和防止覆盖已有配置。

**Windows 固件构建也已验证：**独立安装 ESPHome 2026.10.2、匹配的 esptool 5.4.0，重新下载 Windows ESP-IDF 5.5.6 工具链，从配套源码生成 factory 和 OTA 固件。工程不依赖原来的 Linux 编译环境。

**迁移结论：**提供了可重建环境的源码、依赖版本、相对路径配置和只读自检工具。在满足本教程条件的客户电脑上，可以按相同流程安装运行。已烧好固件的产品换电脑不需要重刷，详见第 12.11 节。

**验证边界：**本次是在同一台 Windows 物理电脑上用新目录、新虚拟环境进行迁移演练，并未访问每台客户电脑。因此不会承诺未知网络、驱动或操作系统组合都免配置运行。新教学固件已在 Windows 编译，未覆盖现有产品固件；核心硬件功能沿用已实机验证版本。客户仍应执行第 10 章的接线和功能验收。完整检查表见 [MIGRATION_VALIDATION.md](MIGRATION_VALIDATION.md)。

### 15.2 【电脑端】参考资料


- [Elecrow 4.2 英寸 Arduino 教程与资源](https://www.elecrow.com/wiki/CrowPanel_ESP32_E-Paper_4.2-inch_Arduino_Tutorial.html#resources)
- [V1.2 版本厂家源码](https://github.com/Elecrow-RD/CrowPanel-ESP32-4.2-E-paper-HMI-Display-with-400-300)
- [ESPHome CLI 安装说明](https://esphome.io/guides/installing_esphome/)
- [ESPHome Native API](https://esphome.io/components/api/)
- [ESPHome GPIO Switch](https://esphome.io/components/switch/gpio/)
- [ESPHome GPIO Binary Sensor](https://esphome.io/components/binary_sensor/gpio/)
- [aioesphomeapi 项目](https://github.com/esphome/aioesphomeapi)
- [esptool 官方文档](https://docs.espressif.com/projects/esptool/en/latest/esp32s3/)

屏幕初始化指令与 GC 波形来源于上述 Elecrow V1.2 版本示例，ESPHome 外部组件封装和电脑本地控制服务为本项目实现。
