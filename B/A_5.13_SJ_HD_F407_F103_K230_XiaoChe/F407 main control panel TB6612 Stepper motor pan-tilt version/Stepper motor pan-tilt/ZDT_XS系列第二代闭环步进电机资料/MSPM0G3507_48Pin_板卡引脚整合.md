# MSPM0G3507 48Pin 板卡引脚整合

> 整理日期：2026-05-15  
> 适用器件：`MSPM0G3507` 48-pin 封装  
> 适用封装：`PT (LQFP-48)` / `RGZ (VQFN-48)`  
> 依据：TI 官方 `MSPM0G3507 Datasheet` 的 `Pin Diagrams` 与 `Pin Attributes`

---

## 1. 先说结论

如果这块板的目标是：

- 一对 ZDT 闭环步进电机
- 做双轴视觉追踪云台
- 控制延迟不能大
- 角度控制要准

那么 `MSPM0G3507` 的板级分配我建议优先按下面思路来画：

- 电机控制主链路：`CAN`
- 电机总线接口：`PA12 + PA13`
- 调试/下载串口：`PA10 + PA11`
- SWD 下载调试：`PA19 + PA20 + NRST`
- IMU 接口：`PA15 + PA16 + PA17`
- 回零/限位输入：`PB17 + PB18`
- 板载状态灯：`PB19 + PB20`
- 同步触发：`PA21 + PA22`
- 扩展/备用通信：`PA26 + PA27`

这套分配的核心优点是：

- `CAN`、`SWD`、`UART`、`I2C` 基本互不打架
- 把默认 `BSL` 相关引脚保留得比较友好，后面排障方便
- 关键控制链路都集中在功能明确的引脚上，后续软件初始化也清晰

---

## 2. 48-pin 封装范围说明

- 本文适用于 `PT` 和 `RGZ` 两种 `48-pin` 版本
- TI datasheet 的 `48 LQFP, VQFN` 在 `Pin Attributes` 表中使用同一列
- 所以下面给出的 `GPIO 名称 + 物理脚号` 可以直接共用

48-pin 版本**没有引出**下面这些 64-pin 才有的脚，画图时不要误抄：

- `PA29`
- `PA30`
- `PB0`
- `PB1`
- `PB4`
- `PB5`
- `PB10`
- `PB11`
- `PB12`
- `PB21`
- `PB22`
- `PB23`
- `PB25`
- `PB26`
- `PB27`

---

## 3. 推荐主分配表

这张表按“现在就要画板”的思路整理，优先给你一份可直接用于原理图网络命名的版本。

| 功能 | MCU 引脚 | 封装脚号 | 引脚类型 | SysConfig 类别 | 推荐网络名 | 说明 |
| --- | --- | --- | --- | --- | --- | --- |
| 电机 CAN TX | `PA12` | `27` | `复用外设` | `MCAN` | `MOTOR_CAN_TXD` | 主推荐，接 CAN 收发器 `TXD` |
| 电机 CAN RX | `PA13` | `28` | `复用外设` | `MCAN` | `MOTOR_CAN_RXD` | 主推荐，接 CAN 收发器 `RXD` |
| CAN 收发器待机/使能 | `PA23` | `43` | `普通GPIO` | `GPIO` | `MOTOR_CAN_STB` | 控制收发器 `STB/EN` |
| 调试串口 TX | `PA10` | `18` | `复用外设` | `UART` | `DBG_UART_TX` | 同时兼容默认 `BSL UART TX` |
| 调试串口 RX | `PA11` | `19` | `复用外设` | `UART` | `DBG_UART_RX` | 同时兼容默认 `BSL UART RX` |
| 上位控制串口 TX | `PB15` | `25` | `复用外设` | `UART` | `HOST_UART_TX` | 可接视觉主控/上位板 |
| 上位控制串口 RX | `PB16` | `26` | `复用外设` | `UART` | `HOST_UART_RX` | 可接视觉主控/上位板 |
| IMU I2C SCL | `PA15` | `30` | `复用外设` | `I2C` | `IMU_I2C_SCL` | 推荐接陀螺仪/磁编码器 |
| IMU I2C SDA | `PA16` | `31` | `复用外设` | `I2C` | `IMU_I2C_SDA` | 推荐接陀螺仪/磁编码器 |
| IMU 中断/数据就绪 | `PA17` | `32` | `普通GPIO` | `GPIO` | `IMU_DRDY` | 推荐接 `INT/DRDY` |
| Boot/恢复按键 | `PA18` | `33` | `普通GPIO` | `GPIO` | `BSL_INVOKE_KEY` | 默认 `BSL invoke`，建议做按键或测试点 |
| SWDIO | `PA19` | `34` | `调试专用` | `DEBUGSS` | `SWDIO` | 下载调试 |
| SWCLK | `PA20` | `35` | `调试专用` | `DEBUGSS` | `SWCLK` | 下载调试 |
| 复位 | `NRST` | `4` | `调试专用` | `SYSCTL` | `NRST` | 调试下载/手动复位 |
| Yaw 回零/限位 | `PB17` | `36` | `普通GPIO` | `GPIO` | `YAW_HOME_IN` | 推荐加上拉和输入滤波 |
| Pitch 回零/限位 | `PB18` | `37` | `普通GPIO` | `GPIO` | `PITCH_HOME_IN` | 推荐加上拉和输入滤波 |
| 运行灯 | `PB19` | `38` | `普通GPIO` | `GPIO` | `LED_RUN` | 板载状态指示 |
| 故障灯 | `PB20` | `41` | `普通GPIO` | `GPIO` | `LED_ERR` | 板载状态指示 |
| 同步触发输入 | `PA21` | `39` | `普通GPIO` | `GPIO` | `SYNC_IN` | 可接相机同步/外部触发 |
| 同步触发输出 | `PA22` | `40` | `普通GPIO` | `GPIO` | `SYNC_OUT` | 可做帧同步/时间戳输出 |
| 电机电源或外设使能 | `PA24` | `44` | `普通GPIO` | `GPIO` | `MOTOR_PWR_EN` | 控制外设上电或使能 |
| 外部故障输入 | `PA25` | `45` | `普通GPIO` | `GPIO` | `FAULT_IN` | 可接急停/外部 fault |
| 备用通信 TX | `PA26` | `46` | `复用外设` | `UART` | `AUX_UART_TX` | 备用；也可改成 CAN 备选 TX |
| 备用通信 RX | `PA27` | `47` | `复用外设` | `UART` | `AUX_UART_RX` | 备用；也可改成 CAN 备选 RX |

类型说明：

- `复用外设`：当前建议把该脚切到 `CAN/UART/I2C` 等片上外设功能
- `普通GPIO`：当前建议按普通输入/输出使用，也可以挂中断
- `调试专用`：当前建议固定保留给 `SWD/NRST`，不要再混挂普通功能

SysConfig 类别说明：

- `MCAN`：在左侧 `MCAN` 分类里添加并配置
- `UART`：在左侧 `UART` 分类里添加并配置
- `I2C`：在左侧 `I2C` 分类里添加并配置
- `GPIO`：在左侧 `GPIO` 分类里添加并配置
- `DEBUGSS`：在左侧 `DEBUGSS` 分类里处理调试相关引脚
- `SYSCTL`：在左侧 `SYSCTL` 分类里处理 `NRST`、时钟等系统脚

---

## 4. 为什么我推荐这样分

### 4.1 CAN 用 `PA12 + PA13`

这是我对这块板的**主推荐**：

- 这对脚本身支持 `CAN_TX / CAN_RX`
- 不会和 `SWD` 冲突
- 也不占用 `PA10/PA11` 这组默认 `BSL UART`
- 适合把 `CAN` 做成主控制总线，后面一条总线带两台电机

`PA26 + PA27` 也能走 CAN，但我更建议把它们留成：

- 备用串口
- 备用 `RS485/TTL`
- 或者做协议排障口

这样板卡后期调试更灵活。

### 4.2 调试口用 `PA10 + PA11`

这两个脚很适合固定成调试 UART：

- 它们本身就是常用串口脚
- 还是默认 `BSL UART TX/RX`
- 后面真遇到固件恢复、串口日志、参数打印，会省很多事

建议直接留一个 4Pin 或 6Pin 调试座：

- `3V3`
- `GND`
- `DBG_UART_TX`
- `DBG_UART_RX`
- 可选 `NRST`

### 4.3 IMU 用 `PA15 + PA16 + PA17`

视觉追踪云台里，IMU 基本是高概率会加的：

- `PA15` 做 `I2C SCL`
- `PA16` 做 `I2C SDA`
- `PA17` 做 `IMU_DRDY / INT`

这样软件结构比较顺：

- 一条 `I2C` 读姿态
- 一条中断线做数据到达同步
- 后面姿态融合和控制周期更容易收敛

### 4.4 `PA18` 不建议乱占

`PA18` 是这里最值得单独提醒的脚之一：

- 它带默认 `BSL invoke`
- 不建议直接绑到一个会在上电复位阶段主动拉高/拉低的复杂外设

最稳妥的用法是：

- 做 `Boot/恢复按键`
- 或者只留测试点
- 或者做一个弱上下拉后再接按键

---

## 5. 画原理图时必须单独留出来的关键脚

### 5.1 电源相关

| 引脚 | 封装脚号 | 说明 |
| --- | --- | --- |
| `VDD` | `6` | `3.3V` 主供电 |
| `VSS` | `7` | 地 |
| `VCORE` | `48` | 内核电源去耦专用，不要外带负载 |

注意：

- `VCORE` 不是普通 `3.3V` 输出脚
- 它只能按 TI 官方硬件要求接去耦
- 不要把别的芯片挂到 `VCORE`

### 5.2 下载调试口

建议原理图上固定保留 `SWD` 接口：

- `SWDIO` -> `PA19`
- `SWCLK` -> `PA20`
- `NRST`
- `3V3`
- `GND`

如果板子空间允许，建议直接按标准 5Pin/6Pin 调试口留出。

### 5.3 Boot 恢复相关

建议把下面几项至少留测试点：

- `PA18`：`BSL invoke`
- `PA10`：默认 `BSL UART TX`
- `PA11`：默认 `BSL UART RX`

如果你后面担心串口恢复不够，再额外预留：

- `PA0`：默认 `BSL I2C SDA`
- `PA1`：默认 `BSL I2C SCL`

---

## 6. 需要谨慎使用的引脚

### 6.1 `PA18`

- 默认带 `BSL invoke`
- 最好不要直接接到强驱动外设
- 最适合做按键、拨码或测试点

### 6.2 `PA10 + PA11`

- 默认带 `BSL UART`
- 很适合保留成调试口
- 不建议再拿去挂 LED、蜂鸣器、功率器件使能之类的低价值功能

### 6.3 `PA3 + PA4`

- 这组脚有 `LFXIN / LFXOUT` 属性
- 如果你后面想加低速晶振，原理图阶段就要先留余地

### 6.4 `PA5 + PA6`

- 这组脚有 `HFXIN / HFXOUT` 属性
- 如果你后面想加高速外部时钟/晶振，这组脚不要先随手占掉

### 6.5 `PA26 + PA27`

- 它们也支持 `CAN`
- 但因为芯片本身只有一套 `CAN` 外设
- 所以这对脚更适合拿来做备用通信口，而不是再并一个重复 CAN 口

---

## 7. 推荐的板级接口清单

如果我是按这颗 MCU 来画这块板，我会固定留下面这些连接器或测试点：

- `SWD` 下载口
- `DBG_UART` 调试口
- `CAN` 总线口
- `IMU` 插座或焊盘
- `YAW_HOME_IN`
- `PITCH_HOME_IN`
- `SYNC_IN`
- `SYNC_OUT`
- `BSL_INVOKE_KEY`
- `FAULT_IN`

这样后面无论你走：

- 纯 CAN 电机控制
- IMU 闭环稳定
- 视觉主控 + 下位机板通信
- 固件恢复 / 在线排障

都不会被板级资源卡住。

---

## 8. 48-pin 物理脚速查表

这张表是“封装脚号 -> 引脚名 -> 快速备注”的速查版，方便你画符号或核对网络表。

| 脚号 | 引脚名 | 快速备注 |
| --- | --- | --- |
| `1` | `PA0` | `I2C0_SDA`；默认 `BSL I2C SDA`；5V 容忍开漏 |
| `2` | `PA1` | `I2C0_SCL`；默认 `BSL I2C SCL`；5V 容忍开漏 |
| `3` | `PA28` | 可作 `UART0_TX / I2C0_SDA` 备选 |
| `4` | `NRST` | 复位脚 |
| `5` | `PA31` | 可作 `UART0_RX / I2C0_SCL` 备选 |
| `6` | `VDD` | `3.3V` 电源 |
| `7` | `VSS` | 地 |
| `8` | `PA2` | `ROSC` / GPIO |
| `9` | `PA3` | `LFXIN` / GPIO |
| `10` | `PA4` | `LFXOUT` / GPIO |
| `11` | `PA5` | `HFXIN` / GPIO |
| `12` | `PA6` | `HFXOUT` / GPIO |
| `13` | `PA7` | GPIO / 定时器输出 |
| `14` | `PB2` | GPIO / `UART3` / `I2C1` |
| `15` | `PB3` | GPIO / `UART3` / `I2C1` |
| `16` | `PA8` | GPIO / `UART1_TX` |
| `17` | `PA9` | GPIO / `UART1_RX` / `RTC_OUT` |
| `18` | `PA10` | `UART0_TX`；默认 `BSL UART TX` |
| `19` | `PA11` | `UART0_RX`；默认 `BSL UART RX` |
| `20` | `PB6` | GPIO / `UART1` / `SPI1` |
| `21` | `PB7` | GPIO / `UART1` / `SPI1` |
| `22` | `PB8` | GPIO / `UART1` / `SPI1` |
| `23` | `PB9` | GPIO / `UART1` / `SPI1` |
| `24` | `PB14` | GPIO / `SPI1` / 定时器 |
| `25` | `PB15` | GPIO / `UART2_TX` / `SPI1` |
| `26` | `PB16` | GPIO / `UART2_RX` / `SPI1` |
| `27` | `PA12` | `CAN_TX` 主推荐 |
| `28` | `PA13` | `CAN_RX` 主推荐 |
| `29` | `PA14` | GPIO / `UART3` / 模拟功能 |
| `30` | `PA15` | `I2C1_SCL`；也可作 `DAC_OUT` |
| `31` | `PA16` | `I2C1_SDA`；也可作模拟输出相关功能 |
| `32` | `PA17` | GPIO / `UART1_TX` / `I2C1_SCL` |
| `33` | `PA18` | GPIO / `UART1_RX` / `I2C1_SDA`；默认 `BSL invoke` |
| `34` | `PA19` | `SWDIO` |
| `35` | `PA20` | `SWCLK` |
| `36` | `PB17` | GPIO / `UART2_TX` / `SPI0` |
| `37` | `PB18` | GPIO / `UART2_RX` / `SPI0` |
| `38` | `PB19` | GPIO / `SPI0` / 流控相关功能 |
| `39` | `PA21` | GPIO / `UART2_TX` / 定时器 / 模拟功能 |
| `40` | `PA22` | GPIO / `UART2_RX` / 定时器 / 模拟功能 |
| `41` | `PB20` | GPIO / `SPI0` / `SPI1` |
| `42` | `PB24` | GPIO / `SPI0` / 定时器 |
| `43` | `PA23` | GPIO / `UART3_CTS` / 模拟功能 |
| `44` | `PA24` | GPIO / `UART3_RTS` / 模拟功能 |
| `45` | `PA25` | GPIO / `UART3_RX` / 模拟功能 |
| `46` | `PA26` | 备用 `CAN_TX` / GPIO / 模拟功能 |
| `47` | `PA27` | 备用 `CAN_RX` / GPIO / 模拟功能 |
| `48` | `VCORE` | 内核电源去耦专用 |

---

## 9. 我对这块板的实操建议

如果你现在就开始画原理图，我建议直接按下面这个优先级做：

1. 先把 `CAN + SWD + DBG_UART + IMU_I2C + 两路 HOME` 固定下来
2. 再把 `SYNC_IN/SYNC_OUT` 留出来
3. 最后把 `PA26/PA27` 当成协议排障口或备用串口

也就是说，先保证：

- 电机能控
- 姿态能读
- 程序能刷
- 问题能查

然后再去做那些“以后可能会用到”的扩展功能。

---

## 10. 官方资料链接

- [TI MSPM0G3507 产品页](https://www.ti.com/product/MSPM0G3507)
- [TI MSPM0G3507 Datasheet PDF](https://www.ti.com/lit/ds/symlink/mspm0g3507.pdf)

---

## 11. 一句话版结论

对于你这块“视觉追踪双轴云台控制板”，`MSPM0G3507 48-pin` 最稳的画法就是：

> `PA12/PA13` 跑电机 `CAN`，`PA10/PA11` 留调试串口，`PA19/PA20` 留 `SWD`，`PA15/PA16/PA17` 给 `IMU`，`PB17/PB18` 给回零，`PA18` 单独当 `Boot/恢复` 用。
