# F407 USART6 接入 F103 运行态短帧记录

- 日期：2026-05-14
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`implemented in code`
- 范围：`F407 main control panel\code\AAA_F407VET6_5.6_Test`

## 1. 本次变更目标

把 `F407` 主控侧真正接上 `F103` 当前运行中的 6 字节短帧：

```text
AA 55 TURN SCENE FLAGS CRC8
```

本次目标不涉及：
- 电机控制
- 舵机控制
- 视觉融合决策
- 上层任务状态机

只处理一件关键基础工作：
- 让 `USART6` 能稳定接收并解析 `F103` 的运行态短帧

## 2. 本次实际修改文件

修改：
- `Core/Inc/gray_module.h`
- `Core/Src/gray_module.c`
- `Core/Src/main.c`
- `Core/Src/usart.c`
- `Core/Inc/stm32f4xx_it.h`
- `Core/Src/stm32f4xx_it.c`

## 3. 本次接入的主链路

### 3.1 接收口

- 使用 `USART6`
- 引脚：`PC6 / PC7`
- 命名：`CK_HD_TX / CK_HD_RX`

### 3.2 接收方式

采用：
- `HAL_UART_Receive_IT()`
- 单字节中断接收

也就是说，当前 `F407` 会在 `USART6` 上逐字节收流，再在主控侧拼帧。

## 4. `gray_module` 新增的运行态能力

本次在 `gray_module` 中新增了 `F103` 运行态短帧对应的数据定义：

### 4.1 新增短帧常量

- `GRAY_MODULE_RUNTIME_FRAME_LEN = 6`
- `GRAY_MODULE_UART_SOF1 = 0xAA`
- `GRAY_MODULE_UART_SOF2 = 0x55`

### 4.2 新增运行态标志位定义

与 `F103` 当前短帧定义保持一致：
- `VALID`
- `LINE_FOUND`
- `LEFT_FEATURE`
- `RIGHT_FEATURE`
- `CENTER_ON_LINE`
- `WIDE_BLACK`
- `UNSTABLE`
- `SENSOR_FAULT`

### 4.3 新增运行态场景定义

与 `F103` 当前短帧定义保持一致：
- `STRAIGHT`
- `SOFT_LEFT`
- `SOFT_RIGHT`
- `HARD_LEFT`
- `HARD_RIGHT`
- `CROSS`
- `LEFT_BRANCH`
- `RIGHT_BRANCH`
- `T_JUNCTION`
- `STOP_BAR`
- `LINE_LOST`
- `AMBIGUOUS`

### 4.4 新增主控侧运行态缓存

`GrayModuleState` 现在除了保留旧版 `status` 外，又新增：
- `runtime.turn`
- `runtime.scene`
- `runtime.flags`

同时加入了串口拼帧需要的：
- `rx_index`
- `rx_buffer[6]`

## 5. 本次新增的解析路径

### 5.1 帧级解析

新增：
- `GrayModule_ParseRuntimeFrame()`

职责：
- 检查帧头 `AA 55`
- 对 `TURN + SCENE + FLAGS` 计算 `CRC8`
- 校验通过后写入 `GrayModuleState.runtime`
- 更新 `last_update_ms`
- 将模块标记为 `online`

### 5.2 字节流拼帧

新增：
- `GrayModule_ConsumeRuntimeUartByte()`

职责：
- 在主控侧逐字节吸收 `USART6` 数据流
- 自动等待 `0xAA`
- 自动匹配第二个帧头 `0x55`
- 收满 6 字节后触发短帧解析
- 解析完成后自动回到等待下一帧状态

这意味着主控已经具备最基本的串口流式收帧能力。

### 5.3 在线状态维护

新增：
- `GrayModule_Service()`

职责：
- 根据 `last_update_ms` 和超时时间维护 `online`

当前在 `main.c` 中采用的超时阈值为：
- `100 ms`

## 6. `main.c` 本次接入内容

本次在 `main.c` 中补上了三件事：

1. 初始化后启动 `USART6` 中断接收
2. 在主循环中周期调用 `GrayModule_Service()`
3. 通过 `HAL_UART_RxCpltCallback()` 把收到的字节喂给 `gray_module`

也就是说，主循环虽然仍然非常轻，但已经不再是空循环。

## 7. 中断链本次接入内容

为了让 `USART6` 真的能进中断，本次同时补上了：

### 7.1 NVIC 使能

在 `usart.c` 的 `USART6_MspInit` 中新增：
- `HAL_NVIC_SetPriority(USART6_IRQn, 0, 0)`
- `HAL_NVIC_EnableIRQ(USART6_IRQn)`

并在 `DeInit` 中补上：
- `HAL_NVIC_DisableIRQ(USART6_IRQn)`

### 7.2 IRQ Handler

在 `stm32f4xx_it.c` 中新增：
- `USART6_IRQHandler()`

内部调用：
- `HAL_UART_IRQHandler(&huart6)`

这样，`HAL_UART_RxCpltCallback()` 才有真正被触发的链路。

## 8. 当前阶段形成的系统语义

截至本次接入完成后，`F407` 对 `F103` 的理解已经发生了一个重要转变：

过去代码骨架更偏向：
- 把 `F103` 当作一块会提供原始灰度状态的外设

本次接入后更明确变成：
- 把 `F103` 当作一块会给出运行态转向建议和场景判断的智能灰度模块

这与当前整个项目的系统分工是一致的。

## 9. 已完成校验

已完成：
- 主机端 `gcc -fsyntax-only` 语法检查

结果：
- 通过
- 仅出现 CMSIS 在主机编译环境下常见的指针宽度 warning
- 本次新增业务代码未出现语法错误

## 10. 当前还没做的事情

本次有意没有进入以下层级：

- 没有把 `TURN` 接到电机/舵机输出
- 没有把 `SCENE` 接到行为层状态机
- 没有把 `F103` 与 `OpenMV` 做融合
- 没有做主控侧调试串口透传

这些都属于下一阶段工作。

## 11. 下一步最自然的收口

现在最自然的下一步应该是上板联调，重点确认：

1. `F407 USART6` 是否确实能收到 `F103` 短帧
2. `GrayModuleState.runtime.turn` 左右符号是否正确
3. `scene / flags` 是否与 `F103` 当前实测一致
4. `online` 超时行为是否符合预期

等这一步稳定之后，再进入主控控制层接入，会顺很多。
