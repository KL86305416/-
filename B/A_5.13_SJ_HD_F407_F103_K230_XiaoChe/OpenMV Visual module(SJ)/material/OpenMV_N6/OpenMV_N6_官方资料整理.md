# OpenMV N6 官方资料整理

- 整理日期：2026-05-20
- 整理目录：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe\OpenMV Visual module(SJ)\material\OpenMV_N6`
- 目标：把 OpenMV N6 的官方页面、官方 PDF、关键图片和一份中文索引整理到本地，方便后续画板、接线、查接口、查器件。

## 1. 官方来源

- Quick Reference：`https://docs.openmv.io/openmvcam/quickref/openmv-n6.html`
- 产品页：`https://openmv.io/products/openmv-n6`
- 官方附件来源：OpenMV 产品页中挂载的官方 PDF 附件（`cdn.shopify.com`）
- 官方 GitHub 媒体文件：OpenMV 官方仓库 `openmv-media` 中的 N6 参考手册 PDF

说明：
- 本目录中的页面、图片、PDF 都来自 OpenMV 官方页面或 OpenMV 官方挂载附件。
- `stm32n657a0.pdf`、`rm0486...pdf` 等虽然内容本身来自 ST，但这里采用的是 OpenMV 官方页面给出的链接或 OpenMV 官方仓库给出的文件入口。

## 2. 已整理到本地的资料概览

- 官方页面：2 份
- 官方图片：2 份
- 核心文档：5 份
- 板上器件数据手册：23 份
- 认证资料：3 份

## 3. OpenMV N6 核心结论

### 3.1 板卡定位

OpenMV N6 是一块偏高性能的嵌入式视觉模组，核心是 `STM32N657`，并配合板载 `PAG7936` 全局快门图像传感器。它的资料结构比较完整，既有快速上手页面，也有整板原理图、图像传感器原理图、MCU 数据手册、参考手册，以及板上关键器件的数据手册。

### 3.2 主要规格

根据 OpenMV 官方 quick reference 与产品页：

- MCU：`STM32N657`，`Cortex-M55 @ 800 MHz`
- NPU：片上 `1 GHz NPU`，官方页面标注 `600 GOPS INT8`
- 内存：`64 MB SDRAM`
- 片上 SRAM：`4.2 MB internal SRAM`
- 板载程序/存储 Flash：`32 MB octal flash`
- 图像传感器：`PAG7936 1MP color global shutter`
- 摄像头性能：官方产品页给出 `1280x800 @ 120 FPS`，`640x400 @ 240 FPS`，`320x200 @ 480 FPS`
- USB：`USB-C High Speed 480 Mb/s`
- 存储：`microSD` 卡槽
- 无线：`Wi-Fi a/b/g/n` + `Bluetooth 5.1`
- 有线网络：`10/100/1000 Ethernet`
- 其他板载器件：`IMU`、`麦克风`、用户按键、RGB LED、电源状态 LED

### 3.3 电源与电平注意事项

这些点对后面接主控、画板、做扩展最重要：

- OpenMV 官方产品页明确强调：`N6 的 I/O 不是 5V tolerant`
- 所有外部逻辑接口应按 `3.3V` 逻辑设计
- 官方说明强调：`应通过 VIN 给板子供电`
- `3.3V` 引脚是板上稳压输出，不应当作为外部输入供电口使用
- `VIN` 官方给出的输入范围重点是 `4.7V ~ 5.7V`
- 支持 `3.7V LiPo` 电池，并带充电管理
- 产品页给出的整板功耗：`Full Power 约 150mA @ 5V`

### 3.4 对接口设计最有价值的 quick reference 信息

Quick reference 里已经给出了非常关键的总线映射，后面做接口分配时建议优先看它：

- UART：
  - `UART3`: `TX=P4`, `RX=P5`
  - `UART4`: `TX=P2`, `RX=P3`
  - `UART7`: `TX=P14`, `RX=P13`
- I2C：
  - `I2C2`: `SCL=P4`, `SDA=P5`
- SPI：
  - `SPI2`: `MOSI=P0`, `MISO=P1`, `SCK=P2`, `CS=P3`
  - `SPI4`: `MOSI=P18`, `MISO=P17`, `SCK=P16`, `CS=P15`
- CAN：
  - Quick reference 当前明确写出的总线是 `CAN1: TX=P2, RX=P3`
  - 产品页宣传文案写的是 `two CAN buses`
  - 因此后续如果你要用 CAN，建议以 quick reference + 原理图双重交叉确认，避免只看宣传页文字
- PWM：
  - 可用 PWM 管脚包括 `P4`、`P5`、`P6`、`P7`、`P8`、`P9`、`P10`
- ADC：
  - `P6_ADC`：满量程约 `3.3V`
  - `BAT_ADC`：内部电池检测通道，满量程约 `5.0V`

### 3.5 对你后面系统集成的意义

如果你后面是把 OpenMV N6 作为视觉模块，接到你自己的 F407 主控板上，这里最值得先记住的结论是：

- 逻辑电平必须严格按 `3.3V` 处理
- 供电不要直接拿 `3.3V` 去喂整板，应该按官方建议走 `VIN`
- 如果只是做视觉结果回传，`UART` 是最直接的
- 如果追求更稳定总线、抗干扰、距离更宽松，可以考虑 `CAN`
- 如果要高吞吐图像/数据交换，再进一步考虑 `SPI`
- 具体到底选 `UART / CAN / SPI`，最好结合你主控板可用 IO、数据量、实时性要求，再回看 quick reference 与原理图一起定

## 4. 建议阅读顺序

建议你按下面顺序使用这些资料：

1. 先看 `official_pages/openmv-n6-quickref.html`
2. 再看 `official_pdfs/core_docs/OpenMV-N6-Schematic-Rev4.pdf`
3. 再看 `official_pdfs/core_docs/pag7936-schematic-v3.pdf`
4. 然后看 `official_pdfs/core_docs/stm32n657a0.pdf`
5. 如果需要深入寄存器、时钟、外设细节，再看 `official_pdfs/core_docs/rm0486-stm32n647657xx-reference-manual.pdf`
6. 如果要查供电链路、Wi-Fi/蓝牙、IMU、ESD/EMI、防护器件，再回到 `component_datasheets` 目录逐个查

## 5. 本地目录结构

```text
OpenMV_N6
|-- OpenMV_N6_官方资料整理.md
|-- official_examples
|   `-- scripts/examples/  (388 .py files, from openmv/openmv)
|-- official_media
|   |-- openmv-n6-hero.jpg
|   `-- pinout-openmv-n6-pag7936.png
|-- official_pages
|   |-- openmv-n6-product-page.html
|   `-- openmv-n6-quickref.html
|-- official_doc_repo
|   |-- N6_related_paths.txt
|   `-- openmv-doc/  (from openmv/openmv-doc)
`-- official_pdfs
    |-- core_docs
    |-- component_datasheets
    `-- certifications
```

## 6. 本地文件清单

### 6.1 官方页面

| 文件 | 用途 |
|---|---|
| `official_pages/openmv-n6-quickref.html` | OpenMV N6 快速参考页，适合先查引脚、总线、供电、运行说明 |
| `official_pages/openmv-n6-product-page.html` | OpenMV N6 产品页，适合查总体规格、官方附件入口、宣传参数 |

### 6.2 官方图片

| 文件 | 用途 |
|---|---|
| `official_media/openmv-n6-hero.jpg` | 官方展示图 |
| `official_media/pinout-openmv-n6-pag7936.png` | 官方引脚图，后面做接线时很有用 |

### 6.3 核心文档

| 文件 | 用途 |
|---|---|
| `official_pdfs/core_docs/Battery_Guide_500mA_Charging.pdf` | 官方电池/充电指导 |
| `official_pdfs/core_docs/OpenMV-N6-Schematic-Rev4.pdf` | OpenMV N6 整板原理图，最关键 |
| `official_pdfs/core_docs/pag7936-schematic-v3.pdf` | PAG7936 摄像头模块/传感器相关原理图 |
| `official_pdfs/core_docs/stm32n657a0.pdf` | STM32N657A0 数据手册 |
| `official_pdfs/core_docs/rm0486-stm32n647657xx-reference-manual.pdf` | STM32N647/657 参考手册，寄存器级开发最关键 |

### 6.4 板上器件数据手册

这些文件基本都是 OpenMV 官方产品页列出来的板上关键器件资料，后面查供电、保护、存储、无线、IMU、音频时会用到：

| 文件 | 说明 |
|---|---|
| `official_pdfs/component_datasheets/PAG7936LT_Product_Brief.pdf` | PAG7936 简版资料 |
| `official_pdfs/component_datasheets/PAG7936LT_Datasheet.pdf` | PAG7936 详细数据手册 |
| `official_pdfs/component_datasheets/BQ24075.pdf` | 电池充电/电源管理相关 |
| `official_pdfs/component_datasheets/LM66200.pdf` | 电源路径/保护相关 |
| `official_pdfs/component_datasheets/LM66100.pdf` | 电源路径/保护相关 |
| `official_pdfs/component_datasheets/TCR2LE18_datasheet_en_20141106.pdf` | LDO/稳压相关 |
| `official_pdfs/component_datasheets/TPS62825.pdf` | DC-DC 相关 |
| `official_pdfs/component_datasheets/AP22800.pdf` | 电源开关相关 |
| `official_pdfs/component_datasheets/MIC5365.pdf` | LDO 相关 |
| `official_pdfs/component_datasheets/APS512XXN_PSRAM_512Mb.pdf` | 外挂 PSRAM 资料 |
| `official_pdfs/component_datasheets/MX25UM25645G_1.8V_256Mb.pdf` | 板载 Flash 资料 |
| `official_pdfs/component_datasheets/TPS22919.pdf` | 负载开关相关 |
| `official_pdfs/component_datasheets/SN74LVC1G3157.pdf` | 模拟开关/信号切换相关 |
| `official_pdfs/component_datasheets/TYPE1YN_Murata_WiFi_BT_Module.pdf` | Murata 无线模组资料 |
| `official_pdfs/component_datasheets/CYW43439_Datasheet.pdf` | Wi-Fi/BT 芯片资料 |
| `official_pdfs/component_datasheets/RTL8211FI-CG.pdf` | 千兆以太网 PHY 资料 |
| `official_pdfs/component_datasheets/PUSB3BB4_SDS.pdf` | ESD/USB 防护器件资料 |
| `official_pdfs/component_datasheets/LSM6DSM.pdf` | 板载 IMU 资料 |
| `official_pdfs/component_datasheets/T3902_microphone.pdf` | 麦克风资料 |
| `official_pdfs/component_datasheets/MCP6006_Data_Sheet.pdf` | 运放资料 |
| `official_pdfs/component_datasheets/PUSB3TB6.pdf` | ESD/USB 防护器件资料 |
| `official_pdfs/component_datasheets/ECMF02-4CMX8.pdf` | EMI/ESD 滤波与防护器件资料 |
| `official_pdfs/component_datasheets/EMI8141-D.pdf` | EMI 相关器件资料 |

### 6.5 认证资料

| 文件 | 说明 |
|---|---|
| `official_pdfs/certifications/OpenMVN6_CE_RED_Certificate.pdf` | CE / RED 认证资料 |
| `official_pdfs/certifications/COBO26010330_ROHS.pdf` | RoHS 资料 |
| `official_pdfs/certifications/COBO26010330_ROHS_2.0.pdf` | RoHS 相关补充资料 |

## 7. 后续最建议你重点看的几个文件

如果你的目标是把 OpenMV N6 作为视觉模组集成到自己的控制系统里，优先级最高的是：

- `official_media/pinout-openmv-n6-pag7936.png`
- `official_pages/openmv-n6-quickref.html`
- `official_pdfs/core_docs/OpenMV-N6-Schematic-Rev4.pdf`
- `official_pdfs/core_docs/stm32n657a0.pdf`
- `official_pdfs/core_docs/rm0486-stm32n647657xx-reference-manual.pdf`

如果你的目标是画扩展板或主控板接口：

- 先看 quick reference 确认引脚功能
- 再看 N6 原理图确认这些引脚在板上到底接了什么
- 再看器件级 datasheet，确认供电、ESD、时序和电平约束

## 8. 我额外帮你确认到的几个关键点

- OpenMV 官方明确提醒：`I/O 不是 5V tolerant`
- `VIN` 不应超过 `5.7V`
- 官方建议通过 `VIN` 供电
- `3.3V` 引脚是输出，不适合作为主供电输入
- quick reference 给出了最实用的外设引脚映射，做主控对接时比单看宣传页更可靠
- 产品页宣传中提到 `two CAN buses`，但 quick reference 当前明确写出的仅有 `CAN1`，这属于后续值得用原理图再核实的一点

## 9. 备注

这份整理的重点是“把官方资料先收齐并分门别类”。如果你后面要继续深入，我可以直接基于这个目录继续帮你做下面几种二次整理：

- 按“和 F407 对接”视角，抽出最相关的引脚与通信方式
- 按“供电树”视角，把 N6 的电源链路单独梳理出来
- 按“扩展板设计”视角，把哪些接口适合引出、哪些需要电平/保护、哪些最好不要碰，单独做一版表

## 10. 官方文档源码仓库入口（2026-05-23 新增）

除了上面这些网页快照、PDF 和图片，本地现在还补充了官方文档源码仓库：

- `official_doc_repo/openmv-doc`

对应来源与提交：

- `openmv/openmv-doc`：`5a5cc691947d08df71203a366bd4c48386474d45`
- `openmv/micropython-doc` 子模块：`e596bf0ac5fe30d539c3f6f4808c0e49129f6efa`

如果你要继续从“文档源码”角度查 N6，最关键的入口是：

- `official_doc_repo/openmv-doc/micropython/docs/openmvcam/quickref/openmv-n6.rst`
- `official_doc_repo/openmv-doc/micropython/docs/openmvcam/tutorial/io_tutorial.rst`
- `official_doc_repo/openmv-doc/micropython/ports/stm32/boards/OPENMV_N6`

我另外生成了一份本地索引：

- `official_doc_repo/N6_related_paths.txt`

这样你后面不需要再从整个文档仓库里手工搜 `N6`。

## 11. 官方例程（2026-05-26 新增）

除了文档、页面和 PDF，本地现在也补充了 OpenMV 官方固件仓库中的**全部例程代码**（MicroPython）。

- `official_examples/`

对应来源与提交：

- `openmv/openmv` 仓库：`99dae3c`（Merge pull request #3159 from openmv/update_micropython）
- 例程路径：`scripts/examples/`
- 总计：**388 个 .py 例程文件**，约 1.5 MB

### 11.1 例程目录结构

| 目录 | 文件数 | 说明 |
|---|---|---|
| `00-HelloWorld/` | 2 | 入门：LED 闪烁、Hello World |
| `01-Camera/` | **73** | 摄像头相关（最常用） |
| `01-Camera/00-Snapshot/` | 5 | 快照：单张拍照、人脸触发、运动触发、延时摄影 |
| `01-Camera/01-Video-Recording/` | 9 | 录像：GIF、MJPEG、人脸触发录制、运动触发录制 |
| `01-Camera/02-Optical-Flow/` | 8 | 光流：位移检测、图像平移 |
| `01-Camera/03-Event-Cameras/` | 20 | 事件相机（Frogeye2020, Genx320） |
| `01-Camera/04-Global-Shutter/` | 2 | 全局快门传感器 |
| `01-Camera/05-Thermal-Cameras/` | 17 | 热成像（FLIR Lepton, FLIR Boson） |
| `01-Camera/06-Time-of-Flight/` | 2 | ToF 测距 |
| `01-Camera/07-Sensor-Control/` | 8 | 传感器参数控制 |
| `01-Camera/08-Readout-Control/` | 2 | 读出控制 |
| `02-Image-Processing/` | **70** | 图像处理 |
| `02-Image-Processing/00-Drawing/` | 19 | 绘图：文字、图形、十字线等 |
| `02-Image-Processing/01-Image-Filters/` | 33 | 滤镜：色彩、边缘检测、二值化、锐化等 |
| `02-Image-Processing/02-Color-Tracking/` | 12 | 颜色追踪：单色、多色、自动曝光 |
| `02-Image-Processing/03-Frame-Differencing/` | 6 | 帧差法：运动检测 |
| `03-Machine-Learning/` | **21** | 机器学习 / AI |
| `03-Machine-Learning/00-TensorFlow/` | 15 | TensorFlow：YOLOv2/v5/v8、人脸检测、手势识别、姿态检测 |
| `03-Machine-Learning/01-ST-CubeAI/` | 1 | ST CubeAI |
| `03-Machine-Learning/02-Haar-Cascade/` | 5 | Haar 级联检测 |
| `04-Barcodes/` | 5 | 条码/二维码识别 |
| `05-Feature-Detection/` | 12 | 特征检测：边缘、角点、直线、圆形、矩形、模板匹配 |
| `06-April-Tags/` | 5 | AprilTag 标记检测 |
| `07-Interface-Library/` | **13** | 接口库 |
| `07-Interface-Library/00-Arduino/` | 3 | Arduino 通信 |
| `07-Interface-Library/01-Pixy-Emulation/` | 6 | Pixy 仿真 |
| `07-Interface-Library/02-MAVLink/` | 2 | MAVLink 协议（无人机） |
| `07-Interface-Library/03-Modbus/` | 2 | Modbus 协议 |
| `08-RPC-Library/` | **6** | RPC 远程调用 |
| `08-RPC-Library/34-Remote-Control/` | 4 | 远程控制（UART/SPI/I2C/CAN） |
| `08-RPC-Library/36-Web-Servers/` | 2 | 内置 Web 服务器 |
| `09-WiFi/` | 3 | Wi-Fi 例程（WINC1500 等） |
| `10-Bluetooth/` | 2 | 蓝牙例程 |
| `11-Open-AMP/` | 1 | 多核通信（Open Asymmetric Multi-Processing） |
| `12-Protocol/` | 5 | 底层协议（I2C/SPI/UART/CAN） |
| `50-Arduino-Boards/` | **91** | Arduino 兼容板专项例程 |
| `50-OpenMV-Boards/` | **66** | OpenMV 板专项例程 |
| `50-OpenMV-Boards/50-STM32-Boards/` | 23 | STM32 系列板控制（含低功耗） |
| `50-OpenMV-Boards/50-IMXRT-Boards/` | 20 | RT1060 系列板控制 |
| `50-OpenMV-Boards/51-Pure-Thermal/` | 4 | Pure Thermal 系列 |
| `50-OpenMV-Boards/52-Alif-Boards/` | 2 | Alif 系列 |
| `50-OpenMV-Boards/60-Shields/` | 17 | 扩展板（LCD、舵机、热电堆、BLE、电机、IMU、距离、TV、光源、触摸屏） |

### 11.2 N6 兼容说明

- N6 是 STM32N6 平台，`index.csv` 中 `Board Type Regex` 为 `.+`（匹配所有）的例程都可在 N6 上运行
- N6 专属的 CSI 摄像头 API（`csi.CSI()`）与旧版 `sensor` 模块不同，01-Camera 中的部分摄像头例程需要根据 quick reference 做适配
- N6 板载 NPU（600 GOPS），`03-Machine-Learning/00-TensorFlow/` 中的 YOLO 等模型例程在 N6 上可发挥最佳性能
- N6 内置 Wi-Fi/BLE，`09-WiFi/` 和 `10-Bluetooth/` 例程可直接使用（无需 WINC1500 等外部模块）
- N6 有千兆以太网，相关网络功能可与 `08-RPC-Library/` 配合

### 11.3 例程兼容性索引

`official_examples/scripts/examples/index.csv` 中记录了每个例程适用的 `Board Type Regex` 和 `Sensor Type Regex`，可以用它快速过滤出 N6（PAG7936 传感器）适用的例程。

### 11.4 更新说明

- 后续如需更新例程到最新版：进入 `official_examples/` 目录，执行 `git pull` 即可
- 当前克隆方式为 `--depth 1 --filter=blob:none --sparse`，只拉取了 `scripts/examples` 目录以节省空间
