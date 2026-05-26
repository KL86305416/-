# F103 串口优先调试记录

- 日期：2026-05-14
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 范围：仅处理 `F103 grayscale sensor(HD)`，暂不继续接入 `F407` 主控业务
- 目标：让 `F103` 自己先完成 8 路灰度采样、板载 LED 指示、`USART1` 周期状态输出

## 1. 本次实现内容

已在 `F103` 工程中接入一个独立的 `gray_app`：

- 周期读取 8 路灰度数字输入
- 生成 `sensor_bits / track_type / edge_count / center_err`
- 镜像到板载 `D1 ~ D8`
- 通过 `USART1` 按统一二进制协议持续发状态帧
- 使用 `KEY1` 切换串口连续发送开关
- 使用 `KEY1-OUT` 指示当前串口状态流是否开启

## 2. 涉及文件

工程路径：
`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe\F103 grayscale sensor(HD)\code\4.28_test`

新增：

- `Core/Inc/gray_app.h`
- `Core/Src/gray_app.c`

接入：

- `Core/Src/main.c`
- `MDK-ARM/4.28_test.uvprojx`

## 3. 当前灰度输入映射

根据原理图/网表，本版按下面顺序采样：

| 通道 | MCU 引脚 | 协议位 |
| --- | --- | --- |
| `IN1` | `PA2` | `bit0` |
| `IN2` | `PA3` | `bit1` |
| `IN3` | `PA4` | `bit2` |
| `IN4` | `PA5` | `bit3` |
| `IN5` | `PB12` | `bit4` |
| `IN6` | `PB13` | `bit5` |
| `IN7` | `PB15` | `bit6` |
| `IN8` | `PA8` | `bit7` |

## 4. 当前板载指示映射

| LED | MCU 引脚 | 显示内容 |
| --- | --- | --- |
| `D1` | `PA15` | `IN8 / bit7` |
| `D2` | `PB3` | `IN7 / bit6` |
| `D3` | `PB4` | `IN6 / bit5` |
| `D4` | `PB5` | `IN5 / bit4` |
| `D5` | `PB6` | `IN4 / bit3` |
| `D6` | `PB7` | `IN3 / bit2` |
| `D7` | `PB8` | `IN2 / bit1` |
| `D8` | `PB9` | `IN1 / bit0` |

附加按键：

| 名称 | 引脚 | 作用 |
| --- | --- | --- |
| `KEY1` | `PA11` | 切换串口连续发送开/关 |
| `KEY1-OUT` | `PB14` | 串口连续发送状态指示 |

## 5. 当前串口行为

### 5.1 串口物理口

- `USART1`
- `PA9 = TX`
- `PA10 = RX`
- 波特率：`115200`
- 格式：`8N1`

### 5.2 上电默认行为

- 默认开启串口状态流
- 上电后立即发送 1 帧状态
- 之后每 `20 ms` 发送 1 帧

### 5.3 按键行为

- 按下 `KEY1` 一次：切换连续发送开/关
- `PB14` 亮：连续发送开启
- `PB14` 灭：连续发送关闭

## 6. 当前协议输出

输出仍沿用之前定的统一帧：

- `SOF1 = 0xAA`
- `SOF2 = 0x55`
- `VER = 0x01`
- `SRC = 0x20`（F103）
- `TYPE = 0x10`（GRAY_STATUS）
- `LEN = 8`
- `CRC16 = CCITT-FALSE`

`GRAY_STATUS` payload：

1. `status_flags`
2. `sensor_bits`
3. `track_type`
4. `edge_count`
5. `center_err low`
6. `center_err high`
7. `frame_cnt low`
8. `frame_cnt high`

## 7. 当前算法说明

本版先用简单、可验证的规则做第一轮落地：

- `sensor_bits`
  - 当前假设比较器输出为低有效
  - 即 GPIO 读到 `RESET` 记为 `1 = 检测到黑线`
- `center_err`
  - 使用 8 路位置加权平均
  - 位置表：`-35 -25 -15 -5 5 15 25 35`
- `edge_count`
  - 使用相邻位跳变次数
- `track_type`
  - `0x00 = NONE`
  - `0x03 = ALL_BLACK`
  - 左端触边优先判 `LEFT_EDGE`
  - 右端触边优先判 `RIGHT_EDGE`
  - 跳变较少判 `SINGLE_LINE`
  - 大面积多点有效判 `CROSS`
  - 其余判 `AMBIGUOUS`

## 8. 当前关键假设

这版为了先把板跑起来，做了一个明确假设：

- 灰度数字输出是“低有效”

同时，基于上板观察，当前又补了一个独立显示假设：

- 板载 `D1 ~ D8` 的“期望显示方向”需要与当前逻辑反相
- 因此代码中已把：
  - `GRAY_SENSOR_ACTIVE_IS_LOW`
  - `GRAY_LED_ACTIVE_IS_LOW`
  拆开独立控制
- 目前设置为：
  - `GRAY_SENSOR_ACTIVE_IS_LOW = 1U`
  - `GRAY_LED_ACTIVE_IS_LOW = 1U`

这表示：

- 串口 `sensor_bits` 语义暂时保持不变
- 只把板载 LED 的显示亮灭反过来

补充说明：

- 结合最新上板观察，当前又确认板载 `D1 ~ D8` 的左右物理顺序与最初假设相反
- 因此 LED 显示已按“左右镜像”修正
- 修正范围仅限板载 LED 可视化，不影响 `sensor_bits` 位序，也不影响后续运行态短帧语义

如果上板后发现：

- 黑线时 LED 灭、白底时 LED 亮
- 或串口 `sensor_bits` 逻辑与实际相反

则按下面规则调整 `gray_app.c`：

- 只想改 LED 显示，不改串口语义：
  - 改 `GRAY_LED_ACTIVE_IS_LOW`
- 想让 LED 和串口语义一起翻转：
  - 改 `GRAY_SENSOR_ACTIVE_IS_LOW`

把对应宏从 `1U` 改成 `0U`，重新编译即可。

## 8.1 本次串口抓包解读

你给出的抓包整体是健康的：

- 帧头固定是 `AA 55`
- `VER = 01`
- `SRC = 20`，对应 `F103`
- `TYPE = 10`，对应 `GRAY_STATUS`
- `LEN = 0008`
- 帧序号与帧计数都连续递增

说明：

- 串口链路稳定
- 打包逻辑稳定
- 数据字段顺序没有错

几个典型 payload 例子：

1. `33 00 00 00 00 00`
   - `status_flags = 0x33`
   - `sensor_bits = 0x00`
   - `track_type = 0`
   - `center_err = 0`
   - 表示当前处于“无有效线/背景态”

2. `23 06 01 02 EC FF`
   - `sensor_bits = 0x06`
   - `track_type = 1 = SINGLE_LINE`
   - `edge_count = 2`
   - `center_err = -20`
   - 说明线偏左

3. `23 40 01 02 19 00`
   - `sensor_bits = 0x40`
   - `track_type = 1 = SINGLE_LINE`
   - `center_err = +25`
   - 说明线偏右

4. `23 E0 05 01 19 00`
   - `sensor_bits = 0xE0`
   - `track_type = 5 = RIGHT_EDGE`
   - `center_err = +25`
   - 说明右侧多路同时有效

5. `23 07 04 01 E7 FF`
   - `sensor_bits = 0x07`
   - `track_type = 4 = LEFT_EDGE`
   - `center_err = -25`
   - 说明左侧多路同时有效

结论：

- 当前串口数据不是乱码，而是已经能反映“左偏 / 右偏 / 边缘态 / 空白态”的真实变化
- 现在最主要剩下的是“语义方向微调”，不是通信本身出问题

## 9. 已完成校验

已完成：

- 源文件接入检查
- `uvprojx` 收录检查
- `gcc -fsyntax-only` 语法编译检查

说明：

- 语法编译已通过
- 命令行下仅出现 CMSIS 针对主机编译的指针宽度 warning
- 未发现本次新增业务代码的语法错误

## 10. 建议上板调试步骤

1. 下载当前 F103 固件
2. 接 USB 转串口到 `PA9/PA10`
3. 串口助手设置：
   - `115200`
   - `8N1`
   - 建议打开 `HEX` 显示
4. 上电后观察：
   - `PB14` 是否亮起
   - 串口是否持续输出 `AA 55 ...`
5. 用黑线/白底遮挡各路灰度头，观察：
   - `D1 ~ D8` 是否对应变化
   - 串口 `sensor_bits` 是否同步变化
6. 按下 `KEY1`，观察：
   - `PB14` 是否翻转
   - 串口连续流是否停止/恢复

## 11. 下一步最合理的收口

在 F103 侧先完成这 3 个确认再去碰 F407：

1. 确认 8 路输入极性是否正确
2. 确认 `bit0 ~ bit7` 的左右顺序是否与实际车头方向一致
3. 确认 `center_err` 正负方向是否符合“左负右正”

这三项一旦确认，后面再接 `F407 USART6` 就会非常顺。 
