# 2026-05-23 移除蓝牙功能

## 目标

- 移除项目中的蓝牙调试功能。
- 删除蓝牙底层驱动。
- 清除蓝牙对应的 USART2 串口配置。

## 改动

- 从 `main.c` 移除：
  - `Bluetooth Debug` 菜单项。
  - 蓝牙调试页面和刷新逻辑。
  - 蓝牙接收计数和最后接收字节状态变量。
  - `Bluetooth_Init()` 初始化调用。
- 删除 `Core/Src/bluetooth.c`。
- 删除 `Core/Inc/bluetooth.h`。
- 从 CMake 源文件列表移除 `bluetooth.c`。
- 从 `FindLine.ioc` 清除 `USART2Freq_Value` 及其 `RCC.IPParameters` 条目。

## 当前菜单

- `Line Follow`
- `Servo Debug`

## 验证

- 已执行 `cmake --build build\Debug`。
- 编译通过。
- 已搜索确认当前源码、CMake 和 `.ioc` 中不再包含 `Bluetooth`、`USART2`、`PA2`、`PA3` 等蓝牙串口相关引用。

## 注意

- 历史项目日志中仍会出现蓝牙记录，仅表示过去曾实现过相关功能。
- `build/Debug` 下可能仍保留旧对象文件缓存，不参与当前构建输出。
- 当前工程目录不是 Git 仓库，无法执行提交。
