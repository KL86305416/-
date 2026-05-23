# 项目日志：蓝牙串口改为中断接收加环形缓冲区

- 时间：2026-05-13 16:29
- 改动范围：蓝牙 USART2 接收路径、中断入口

## 问题目标

- 原蓝牙接收逻辑由主循环轮询 USART2 接收标志位，主循环被 OLED 刷新或延时占用时，接收响应会变慢。
- 改为中断接收，让数据到达 USART2 后立即进入中断保存，主循环再从缓冲区读取。

## 改动内容

- 新增 `BLUETOOTH_RX_BUFFER_SIZE`，默认环形缓冲区大小为 `64` 字节。
- 新增蓝牙接收环形缓冲区，使用 head/tail 索引保存 USART2 收到的数据。
- `Bluetooth_ReadByte()` 改为从环形缓冲区取数据，不再直接轮询 USART2 的 `RXNE` 标志。
- `Bluetooth_Init()` 中开启 `USART2_IRQn`，并使能 `USART_CR1_RXNEIE_RXFNEIE` 接收中断。
- 在 `stm32g4xx_it.c` 中新增 `USART2_IRQHandler()`，转发到 `Bluetooth_IRQHandler()` 处理接收中断。
- 初始化和反初始化时会清空接收缓冲区，并正确启用/禁用 USART2 中断。

## 验证结果

- 已执行 `cmake --build build/Debug`，编译和链接通过。
- 构建结果：RAM 使用 `3304 B / 32 KB`，FLASH 使用 `22464 B / 128 KB`。
