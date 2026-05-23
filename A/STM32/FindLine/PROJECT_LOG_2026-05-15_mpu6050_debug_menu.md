# 2026-05-15 MPU6050 调试菜单

## 目标

- 按接线图接入 MPU6050：
  - PC8：I2C3_SCL
  - PC9：I2C3_SDA
  - PA12：MPU6050_INT
  - VCC：+5V
  - GND：GND
- 在主菜单新增 MPU6050 调试功能，显示陀螺仪实时数据。

## 改动

- 新增 `Core/Inc/mpu6050.h`、`Core/Src/mpu6050.c`。
- 新增 I2C3 初始化，使用 PC8/PC9 连接 MPU6050。
- PA12 配置为输入，用于显示 MPU6050 INT 引脚状态。
- 主菜单新增 `MPU6050 Debug` 项。
- 调试页显示：
  - I2C 地址和 WHO_AM_I。
  - GX/GY/GZ 三轴陀螺仪角速度，单位为 dps。
  - 读取计数、INT 引脚状态、数据就绪位和错误计数。
  - 设备未连接或识别失败时显示接线检查提示。

## 验证

- 已执行 `cmake --build build\Debug`。
- 编译通过，生成 `FindLine.elf`。

## 注意

- MPU6050 使用 7 位地址 `0x68`，同时兼容探测 `0x69`。
- 当前工程目录不是 Git 仓库，无法执行本次功能提交。
