# 协议代码骨架接入记录

- 日期：2026-05-14
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`code skeleton integrated`

## 1. 本次目标

基于前面已经确认的硬件拓扑与协议草案，把 `F103` 灰度模块、`F407` 主控、`OpenMV` 视觉模块之间的通信边界先固化为代码骨架，做到：

- 工程里已经有明确的结构体、枚举、寄存器定义、帧定义
- `Keil uvprojx` 已收录新增源文件
- 后续可以直接在这些接口上补真实 I2C/UART 收发与业务逻辑

## 2. 本次新增文件

### 2.1 F103 灰度模块工程

工程路径：
`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe\F103 grayscale sensor(HD)\code\4.28_test`

新增：

- `Core/Inc/gray_protocol.h`
- `Core/Src/gray_protocol.c`

作用：

- 定义 `GrayStatusFrame`
- 定义 I2C 寄存器块地址
- 定义灰度模块命令字
- 提供 UART 状态帧打包
- 提供 CRC16-CCITT-FALSE
- 提供寄存器读写缓存接口

### 2.2 F407 主控工程

工程路径：
`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe\F407 main control panel\code\AAA_F407VET6_5.6_Test`

新增：

- `Core/Inc/gray_module.h`
- `Core/Src/gray_module.c`
- `Core/Inc/vision_protocol.h`
- `Core/Src/vision_protocol.c`

作用：

- `gray_module`
  - 解析 F103 的 I2C 状态块
  - 解析 F103 的 UART 备用状态 payload
  - 生成 4 字节 I2C 命令块
- `vision_protocol`
  - 定义 OpenMV/F407 通用 UART 帧格式
  - 提供建帧、解帧、CRC
  - 缓存 `HELLO / STATUS / VISION_RESULT`

## 3. 已接入主工程的位置

### 3.1 F103

- `main.c` 已加入 `gray_protocol.h`
- `main.c` 初始化阶段已调用 `GrayProtocol_Init()`
- `MDK-ARM/4.28_test.uvprojx` 已加入 `gray_protocol.c`

### 3.2 F407

- `main.c` 已加入 `gray_module.h` 与 `vision_protocol.h`
- `main.c` 初始化阶段已建立：
  - `GrayModuleState`
  - `VisionModuleState`
- `main.c` 初始化阶段已调用：
  - `GrayModule_Init()`
  - `VisionModule_Init()`
- `MDK-ARM/F407VET6_5.1_Test.uvprojx` 已加入：
  - `gray_module.c`
  - `vision_protocol.c`

## 4. 本次已经固化的协议事实

### 4.1 F103 <-> F407

- 主链路：`I2C`
- `F407 = I2C3 Master`
- `F103 = I2C2 Slave`
- 建议地址：`0x42`
- 状态块长度：`10 bytes`
- UART 备用链路：
  - `F407 USART6`
  - `F103 USART1`
  - 源地址：`0x20 = F103`

### 4.2 OpenMV <-> F407

- 物理口：`F407 UART4 <-> OpenMV UART`
- 波特率：`115200 8N1`
- 统一帧头：
  - `0xAA 0x55`
  - `VER = 0x01`
  - 小端 `LEN`
  - `CRC16-CCITT-FALSE`

## 5. 当前还没有接上的部分

这次是“协议骨架接入”，不是“整套联调完成”。以下部分还没有真正落硬件动作：

- `F103` 还没有启用真正的 I2C Slave Listen / 回调收发流程
- `F103` 还没有把 8 路灰度传感器结果真实写入 `GrayStatusFrame`
- `F103` 还没有打开 UART 周期发送任务
- `F407` 还没有在 `UART4 / USART6` 中断或 DMA 中实际喂给解析器
- `F407` 还没有在控制周期里真实轮询 `I2C3` 去读取 F103 状态块
- `F407` 还没有把视觉结果或灰度结果接入小车决策控制

## 6. 这样做的价值

先把协议层独立出来有三个直接好处：

- 后续接真收发时，不需要一边联硬件一边临时定结构体
- `OpenMV` 与 `F103` UART 都复用同一种基础帧，F407 侧逻辑会更统一
- 可以明确区分“通信问题”和“控制策略问题”

## 7. 推荐下一步顺序

建议按下面顺序继续：

1. 先补 `F407 I2C3 -> F103` 的固定 `10` 字节轮询读取
2. 再补 `F103` 侧把真实 `8` 路灰度结果写入 `GrayStatusFrame`
3. 然后补 `F407 UART4` 接收 `OpenMV STATUS / VISION_RESULT`
4. 最后再把灰度与视觉状态接进主控决策

## 8. 本次校验说明

已完成：

- 新文件存在性检查
- `uvprojx` 收录检查
- 源码级一致性自检

未完成：

- 命令行编译校验

原因：

- 当前环境下未找到可直接调用的 `clang / armclang / armcc / uv4`
- 因此还没有做一轮真正的本地编译
