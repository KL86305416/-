# 50-OpenMV-Boards — OpenMV 板卡专项例程

OpenMV 各系列板卡的专项例程，包括 STM32 系列、IMXRT 系列、Pure Thermal 系列、Alif 系列以及各种扩展板（Shields）。

## 50-STM32-Boards（STM32 系列：OpenMV2/3/4/4P/PT）

适用于 OpenMV Cam M4/M7/H7/H7 Plus/PT 等 STM32 系列板卡。

### 50-Board-Control（板载外设控制）

| 例程 | 说明 |
|---|---|
| `adc_read_ext_channel.py` | ADC 读取外部模拟通道 |
| `adc_read_int_channel.py` | ADC 读取内部通道（温度、VREF 等） |
| `can.py` | CAN 总线通信 |
| `cpufreq_scaling.py` | CPU 频率动态调节 |
| `dac_write.py` | DAC 模拟电压输出 |
| `dac_write_timed.py` | DAC 定时波形输出 |
| `i2c_control.py` | I2C 总线控制与扫描 |
| `led_control.py` | 板载 LED 控制 |
| `pin_control.py` | GPIO 引脚输入/输出 |
| `pwm_control.py` | PWM 脉宽调制输出 |
| `rtc.py` | RTC 实时时钟 |
| `servo_control.py` | 舵机控制 |
| `spi_control.py` | SPI 总线直接控制（如 LCD 扩展板） |
| `timer_control.py` | 定时器回调函数 |
| `timer_tests.py` | 定时器全面测试 |
| `uart_control.py` | UART 串口收发 |
| `usb_hid.py` | USB HID 设备模拟（鼠标/键盘） |
| `usb_vcp.py` | USB VCP 虚拟串口图像传输 |
| `vsync_gpio_output.py` | VSYNC 中断触发 GPIO 同步输出 |

### 80-Low-Power（低功耗）

| 例程 | 说明 |
|---|---|
| `deep_sleep.py` | 深度睡眠模式（最低功耗） |
| `stop_mode.py` | 停止模式（保持 RAM，快速唤醒） |
| `extint_wakeup.py` | 外部中断唤醒 |
| `sensor_sleep.py` | 仅传感器睡眠（CPU 保持运行） |

---

## 50-IMXRT-Boards（RT1060 系列：OpenMV RT1060）

适用于基于 NXP i.MX RT1060 的 OpenMV 板卡。

### 50-Board-Control（板载外设控制）

| 例程 | 说明 |
|---|---|
| `adc.py` | ADC 读取模拟引脚 |
| `can.py` | CAN 总线通信 |
| `i2c_control.py` | I2C 总线控制 |
| `led_control.py` | LED 控制 |
| `led_control_pwm.py` | LED PWM 调光控制 |
| `motor_control_dir_pwm.py` | 电机控制（方向 + PWM 模式） |
| `motor_control_hbridge.py` | 电机控制（H 桥模式） |
| `pin_control.py` | GPIO 引脚控制 |
| `power_good_pin.py` | 电源状态引脚读取 |
| `pwm_control.py` | PWM 输出 |
| `rtc.py` | RTC 实时时钟 |
| `servo_control.py` | 舵机控制 |
| `spi_control.py` | SPI 总线直接控制 LCD 扩展板 |
| `switch_pin.py` | 用户按键读取 |
| `timer_control.py` | 定时器回调 |
| `uart_control.py` | UART 串口收发 |
| `watchdog.py` | 看门狗定时器 |

### 80-Low-Power（低功耗）

| 例程 | 说明 |
|---|---|
| `deep_sleep.py` | 深度睡眠 + 传感器关闭 |
| `stop_mode.py` | 停止模式 |
| `sensor_sleep.py` | 传感器睡眠 |

---

## 51-Pure-Thermal（Pure Thermal 系列）

适用于 OpenMV Pure Thermal（纯热成像）板卡。

### 00-HelloWorld（入门）

| 例程 | 说明 |
|---|---|
| `blinky.py` | LED 闪烁入门 |
| `helloworld.py` | Hello World |
| `buzzer.py` | 蜂鸣器控制 |
| `pure_thermal.py` | Pure Thermal 热成像主例程 |

---

## 52-Alif-Boards（Alif 系列：OpenMV AE3）

适用于 OpenMV AE3（Alif Ensemble 系列）板卡。

### 51-OLED-Shield（OLED 扩展板）

| 例程 | 说明 |
|---|---|
| `oled_display.py` | OLED 显示屏基础显示 |
| `oled_display_joystick.py` | OLED + 摇杆控制 |

---

## 60-Shields（OpenMV 扩展板）

适用于各型号 OpenMV 的扩展板/Shield 模块。

### 60-LCD-Shield（LCD 显示屏扩展板）

| 例程 | 说明 |
|---|---|
| `lcd_shield.py` | LCD 扩展板基础显示：帧缓冲实时显示到 LCD |

### 61-Servo-Shield（舵机扩展板）

| 例程 | 说明 |
|---|---|
| `servo.py` | 舵机控制（PCA9685 PWM 驱动） |
| `main.py` | 舵机扩展板完整示例 |
| `pca9685.py` | PCA9685 驱动库 |

### 62-Thermopile-Shield（热电堆热成像扩展板）

| 例程 | 说明 |
|---|---|
| `thermal_camera.py` | 热成像叠加显示 |
| `thermal_overlay.py` | 热成像伪彩色叠加 |
| `thermal_overlay_lcd.py` | 热成像 + LCD 显示 |

### 63-BLE-Shield（蓝牙扩展板）

| 例程 | 说明 |
|---|---|
| `ble.py` | BLE 蓝牙驱动与通信（ASM001/ASM002） |

### 64-Motor-Shield（电机驱动扩展板）

| 例程 | 说明 |
|---|---|
| `motor.py` | 直流电机控制 |
| `stepper.py` | 步进电机控制 |
| `motor-shield-pwm.py` | 电机 PWM 调速 |
| `motor-shield-power-driver.py` | 电机功率驱动 |

### 65-IMU-Shield（IMU 惯性测量扩展板）

| 例程 | 说明 |
|---|---|
| `imu_read.py` | IMU 数据读取（加速度/陀螺仪） |

### 66-Distance-Shield（距离传感器扩展板）

| 例程 | 说明 |
|---|---|
| `distance_read.py` | 红外/激光测距传感器读取 |

### 67-TV-Shield（TV 输出扩展板）

| 例程 | 说明 |
|---|---|
| `tv.py` | TV/显示器视频输出（复合视频信号） |

### 68-Light-Shield（光源扩展板）

| 例程 | 说明 |
|---|---|
| `light.py` | 可控光源（配合 Machine 模块使用） |

### 69-Touch-LCD-Shield（触摸 LCD 扩展板）

| 例程 | 说明 |
|---|---|
| `touch_lcd_shield.py` | 触摸 LCD 显示屏交互（含 N6 适配） |

---

**N6 注意**：
- N6 属于 STM32 系列（STM32N6），`50-STM32-Boards` 中的外设控制例程可参考使用，但引脚映射需查阅 N6 quick reference
- `60-Shields` 中的扩展板能否使用取决于 N6 的物理接口兼容性（N6 引脚排列与旧款 OpenMV 不完全相同）
- `50-IMXRT-Boards`、`51-Pure-Thermal`、`52-Alif-Boards` 例程针对各自的硬件平台，不直接适用于 N6
