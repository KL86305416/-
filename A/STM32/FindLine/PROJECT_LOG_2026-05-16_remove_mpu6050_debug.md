# 2026-05-16 移除 MPU6050 调试功能

## 目标

- 按要求移除 MPU6050 调试功能。
- 将主菜单恢复为巡线、蓝牙调试、舵机调试三项。

## 改动

- 删除 `Core/Inc/mpu6050.h`。
- 删除 `Core/Src/mpu6050.c`。
- 从 `main.c` 移除：
  - `MPU6050 Debug` 菜单项。
  - MPU6050 调试页面。
  - MPU6050 状态变量和刷新逻辑。
  - `MX_I2C3_Init()` 调用。
- 从 `i2c.c` / `i2c.h` 移除 I2C3 初始化和句柄。
- 从 `gpio.c` 移除 PA12 MPU6050 INT 输入配置。
- 从 CMake 源文件列表移除 `mpu6050.c`。

## 保留项

- 保留外部高速晶振 HSE 配置。
- 保留主菜单按键原始状态显示；这部分不属于 MPU6050 功能。

## 验证

- 已执行 `cmake --build build\Debug`。
- 编译通过，生成 `FindLine.elf`。

## 注意

- `FindLine.ioc` 中仍有 `RCC.I2C3Freq_Value`，这是 CubeMX 自动保存的时钟频率字段，不能说明 I2C3 已启用。
- 当前工程目录不是 Git 仓库，无法执行提交。
