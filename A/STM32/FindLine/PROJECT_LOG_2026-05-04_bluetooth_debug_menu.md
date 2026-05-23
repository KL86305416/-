# 项目日志：蓝牙调试菜单与 USART2 驱动

- 时间：2026-05-04 21:29
- 改动范围：蓝牙驱动、主菜单、构建文件

## 改动内容

- 新增 `Core/Inc/bluetooth.h` 和 `Core/Src/bluetooth.c`。
- 蓝牙使用 USART2：
  - PA2：USART2_TX，连接蓝牙 RXD
  - PA3：USART2_RX，连接蓝牙 TXD
  - 默认波特率：`9600`
- 当前工程没有 UART HAL 文件，因此蓝牙驱动使用 USART2 寄存器实现。
- 新增菜单项 `Bluetooth Debug`。
- 蓝牙调试界面功能：
  - 显示 USART2 状态
  - 显示最近收到的字节
  - 统计接收字节数
  - 收到蓝牙数据后自动回显
  - PC0 返回主菜单
- 构建文件已加入 `bluetooth.c`。

## 验证

- 已对 `Core/Src/bluetooth.c` 执行语法检查，通过。
- 已对 `Core/Src/main.c` 执行语法检查，通过。
