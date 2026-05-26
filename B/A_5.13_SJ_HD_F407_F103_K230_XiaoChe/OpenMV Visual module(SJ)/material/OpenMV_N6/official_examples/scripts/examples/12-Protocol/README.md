# 12-Protocol — 底层通信协议例程

MCU 外设通信协议的基础操作例程（I2C、SPI、UART、CAN）。

| 例程 | 说明 |
|---|---|
| `i2c_basic.py` | I2C 基础：主机模式读写外部 I2C 设备 |
| `spi_basic.py` | SPI 基础：主机模式发送/接收数据 |
| `uart_basic.py` | UART 基础：串口收发数据 |
| `can_basic.py` | CAN 总线基础：CAN 报文收发 |
| `gpio_control.py` | GPIO 控制：引脚输入/输出、中断 |

---

**N6 注意**：N6 有丰富的通信外设（3×UART、2×SPI、2×I2C、2×CAN），本目录例程可直接在 N6 上运行，参考 N6 quick reference 的引脚映射。
