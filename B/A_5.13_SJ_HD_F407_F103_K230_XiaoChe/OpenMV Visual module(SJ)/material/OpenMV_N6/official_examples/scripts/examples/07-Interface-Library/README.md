# 07-Interface-Library — 接口库例程

OpenMV 与外部设备/系统通信的接口库，包括 Arduino 通信、Pixy 仿真、MAVLink 和 Modbus 协议。

## 00-Arduino（Arduino 通信）

OpenMV 作为传感器外设与 Arduino 通信。

| 例程 | 说明 |
|---|---|
| `arduino_i2c_slave.py` | I2C 从机模式：OpenMV 作为 I2C 从设备向 Arduino 发送数据 |
| `arduino_spi_slave.py` | SPI 从机模式：OpenMV 作为 SPI 从设备向 Arduino 发送数据 |
| `arduino_uart.py` | UART 通信：OpenMV 通过串口与 Arduino 交换数据 |

## 01-Pixy-Emulation（Pixy 仿真）

模拟 Pixy2 视觉传感器的通信协议，可替代 Pixy2 直接接入现有系统。

| 例程 | 说明 |
|---|---|
| `pixy_uart_emulation.py` | Pixy UART 协议仿真 |
| `pixy_i2c_emulation.py` | Pixy I2C 协议仿真 |
| `pixy_spi_emulation.py` | Pixy SPI 协议仿真 |
| `apriltags_pixy_uart_emulation.py` | AprilTag 检测结果通过 Pixy UART 协议输出 |
| `apriltags_pixy_i2c_emulation.py` | AprilTag 检测结果通过 Pixy I2C 协议输出 |
| `apriltags_pixy_spi_emulation.py` | AprilTag 检测结果通过 Pixy SPI 协议输出 |

## 02-MAVLink（无人机 MAVLink 协议）

与 Pixhawk/ArduPilot/PX4 等飞控系统通信。

| 例程 | 说明 |
|---|---|
| `mavlink_apriltags_landing_target.py` | 发送 AprilTag 检测结果作为 MAVLink 降落目标信息 |
| `mavlink_opticalflow.py` | 发送光流数据作为 MAVLink 光流传感器信息 |

## 03-Modbus（工业 Modbus 协议）

| 例程 | 说明 |
|---|---|
| `modbus_rtu_slave.py` | Modbus RTU 从站：OpenMV 作为 Modbus 从设备 |
| `modbus_apriltag.py` | 通过 Modbus 寄存器输出 AprilTag 检测结果 |

---

**N6 注意**：N6 有 3 路 UART、2 路 SPI、1 路 I2C、CAN 总线，接口丰富，非常适合作为视觉子模块与主控通信。
