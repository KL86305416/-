# 50-Arduino-Boards — Arduino 兼容板专项例程

适用于 Arduino 生态兼容板卡（Giga H7、Portenta H7、Nicla Vision、Nano 33 BLE Sense、Nano RP2040）的专项例程，涵盖板载外设控制、传感器、音频、低功耗等。

## Giga-H7（Arduino Giga H7）

### 50-Board-Control（板载外设控制）

| 例程 | 说明 |
|---|---|
| `adc_read_ext_channel.py` | ADC 读取外部模拟引脚 |
| `adc_read_int_channel.py` | ADC 读取内部通道 |
| `can.py` | CAN 总线通信 |
| `cpufreq_scaling.py` | CPU 频率动态调节 |
| `i2c_scanner.py` | I2C 总线设备扫描 |
| `led_control.py` | RGB LED 控制 |
| `pin_control.py` | GPIO 引脚输入/输出 |
| `pwm_control.py` | PWM 输出控制 |
| `rtc.py` | 实时时钟（RTC） |
| `spi_control.py` | SPI 总线控制 |
| `timer_control.py` | 定时器回调 |
| `uart_control.py` | UART 串口收发 |
| `usb_hid.py` | USB HID 鼠标模拟 |
| `usb_vcp.py` | USB 虚拟串口（VCP）图像传输 |
| `vsync_gpio_output.py` | VSYNC 中断触发 GPIO 输出 |

### 51-Display（显示屏）

| 例程 | 说明 |
|---|---|
| `display.py` | LCD + 触摸屏 + 摄像头显示 |

### 52-Audio（音频）

| 例程 | 说明 |
|---|---|
| `audio_fft.py` | 音频 FFT 频谱分析 |
| `micro_speech.py` | 语音指令识别（MicroSpeech） |

### 53-Low-Power（低功耗）

| 例程 | 说明 |
|---|---|
| `deep_sleep.py` | 深度睡眠模式 + 传感器关闭 |
| `stop_mode.py` | 停止模式（保持 RAM） |
| `extint_wakeup.py` | 外部中断唤醒 |

---

## Nano-33-BLE-Sense（Arduino Nano 33 BLE Sense）

### 10-Bluetooth（蓝牙）

| 例程 | 说明 |
|---|---|
| `ble_blinky.py` | BLE LED 控制（手机 APP 控制板载 LED） |
| `ble_scan.py` | BLE 扫描周围设备 |
| `ble_temperature.py` | BLE 温度传感器广播 |

### 50-Board-Control（板载外设）

| 例程 | 说明 |
|---|---|
| `i2c_scanner.py` | I2C 总线设备扫描 |

### 51-Sensors（传感器）

| 例程 | 说明 |
|---|---|
| `hts221.py` | 温湿度传感器（HTS221 / HS3003） |
| `imu.py` | IMU 惯性测量单元（LSM9DS1） |
| `lps22.py` | 气压传感器 |

### 52-Audio（音频）

| 例程 | 说明 |
|---|---|
| `audio_fft.py` | 音频 FFT 频谱分析 |

### 53-Thermal（热成像）

| 例程 | 说明 |
|---|---|
| `thermal_camera.py` | 热成像叠加（MLX/AMG 低分辨率 FIR 传感器） |

---

## Nano-RP2040（Arduino Nano RP2040 Connect）

### 50-Board-Control

| 例程 | 说明 |
|---|---|
| `i2c_scanner.py` | I2C 总线扫描 |

### 51-Sensors

| 例程 | 说明 |
|---|---|
| `lsm6dsox_basic.py` | LSM6DSOX IMU 基础读取 |
| `lsm6dsox_mlc.py` | LSM6DSOX 机器学习内核（MLC） |

### 52-Audio

| 例程 | 说明 |
|---|---|
| `audio_fft.py` | 音频 FFT |

### 53-Thermal

| 例程 | 说明 |
|---|---|
| `thermal_camera.py` | 热成像叠加 |

---

## Nicla-Vision（Arduino Nicla Vision）

### 50-Board-Control（板载外设）

| 例程 | 说明 |
|---|---|
| `adc_read_ext_channel.py` | ADC 外部通道读取 |
| `adc_read_int_channel.py` | ADC 内部通道读取 |
| `can.py` | CAN 总线通信 |
| `cpufreq_scaling.py` | CPU 频率调节 |
| `i2c_control.py` | I2C 控制 |
| `led_control.py` | RGB LED 控制 |
| `pin_control.py` | GPIO 引脚控制 |
| `pwm_control.py` | PWM 输出 |
| `rtc.py` | RTC 实时时钟 |
| `spi_control.py` | SPI 总线 |
| `timer_control.py` | 定时器回调 |
| `uart_control.py` | UART 串口 |
| `usb_hid.py` | USB HID |
| `usb_vcp.py` | USB VCP 图像传输 |
| `vsync_gpio_output.py` | VSYNC GPIO 输出 |

### 51-Sensors（传感器）

| 例程 | 说明 |
|---|---|
| `lsm6dsox_basic.py` | LSM6DSOX 陀螺仪 |
| `lsm6dsox_mlc.py` | LSM6DSOX MLC 机器学习内核 |
| `vl53l1x_tof.py` | VL53L1X ToF 测距传感器 |

### 52-Audio（音频）

| 例程 | 说明 |
|---|---|
| `audio_fft.py` | 音频 FFT |

### 53-Low-Power（低功耗）

| 例程 | 说明 |
|---|---|
| `deep_sleep.py` | 深度睡眠 |
| `stop_mode.py` | 停止模式 |
| `extint_wakeup.py` | 外部中断唤醒 |

---

## Portenta-H7（Arduino Portenta H7）

### 50-Board-Control（板载外设）

| 例程 | 说明 |
|---|---|
| `adc_read_ext_channel.py` | ADC 外部通道 |
| `adc_read_int_channel.py` | ADC 内部通道 |
| `can.py` | CAN 总线通信 |
| `cpufreq_scaling.py` | CPU 频率调节 |
| `dac_write.py` | DAC 模拟输出 |
| `dac_write_timed.py` | DAC 定时输出 |
| `i2c_control.py` | I2C 控制 |
| `led_control.py` | LED 控制 |
| `pin_control.py` | GPIO 控制 |
| `pwm_control.py` | PWM 输出 |
| `rtc.py` | RTC 实时时钟 |
| `servo_control.py` | 舵机控制 |
| `spi_control.py` | SPI 总线 |
| `timer_control.py` | 定时器回调 |
| `timer_tests.py` | 定时器测试 |
| `uart_control.py` | UART 串口 |
| `usb_hid.py` | USB HID 鼠标 |
| `usb_vcp.py` | USB VCP |
| `vsync_gpio_output.py` | VSYNC GPIO 输出 |

### 51-Audio（音频）

| 例程 | 说明 |
|---|---|
| `audio_fft.py` | 音频 FFT |
| `micro_speech.py` | 语音识别 |

### 52-LoRa（LoRa 无线）

| 例程 | 说明 |
|---|---|
| `lora-example.py` | LoRa 无线通信 |

### 53-Ethernet（以太网）

| 例程 | 说明 |
|---|---|
| `eth_cable_test.py` | 网线连接状态检测 |
| `http_client.py` | HTTP 客户端（LAN） |
| `http_client_ssl.py` | HTTPS/SSL 客户端 |
| `peer_to_peer.py` | 以太网点对点通信 |

### 54-Low-Power（低功耗）

| 例程 | 说明 |
|---|---|
| `deep_sleep.py` | 深度睡眠 |
| `stop_mode.py` | 停止模式 |
| `extint_wakeup.py` | 外部中断唤醒 |
| `sensor_sleep.py` | 传感器睡眠模式 |
| `himax_wakeup_on_motion_detection.py` | Himax 运动检测唤醒 |

---

**N6 注意**：本目录例程针对 Arduino 生态板卡，与 N6 硬件平台不同。但其中的外设操作方式（ADC、PWM、I2C、SPI、UART、CAN、RTC、Timer）可作为 N6 上类似功能的参考。
