# F407 按键点灯自检接入记录
- 日期：2026-05-14
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`implemented and build-verified`
- 范围：`F407 main control panel\code\AAA_F407VET6_5.6_Test`

## 1. 本次目标

为了先验证 `F407` 主控板最基础的：

- GPIO 输入
- GPIO 输出
- 主循环执行

本次加了一条最小自检链路：

- 检测 `D5 / D6`
- 任一按键按下一次
- 点亮 `E0 / B9`

其中：

- `D5 -> PD5 -> KEY_3`
- `D6 -> PD6 -> KEY_4`
- `B9 -> PB9 -> LED_1`
- `E0 -> PE0 -> LED_2`

## 2. 电气方向确认

基于网表文件：

- [F407 main control panel.enet](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/PCB%20schematic%20diagram/F407%20main%20control%20panel.enet)

已经确认：

### 2.1 按键极性

- `PD5` 对应网 `AJ-3`
- `PD6` 对应网 `AJ-4`
- `AJ-3` 通过 `R4 = 10K` 上拉到 `3.3V`
- `AJ-4` 通过 `R5 = 10K` 上拉到 `3.3V`
- 对应按键 `SW4 / SW5` 按下时接地

因此结论是：

- `KEY_3 / KEY_4` 为 **低电平按下**

### 2.2 LED 极性

- `PB9` 经限流电阻后接 `LED1` 阳极
- `PE0` 经限流电阻后接 `LED2` 阳极
- 两颗 LED 阴极接地

因此结论是：

- `LED_1 / LED_2` 为 **高电平点亮**

## 3. 本次代码接入内容

修改文件：

- [main.c](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/Core/Src/main.c:1)

新增内容包括：

1. 新增自检状态变量
   - `g_key_led_latched`
   - `g_key_any_prev_pressed`
2. 新增初始化函数
   - `KeyLedSelfTest_Init()`
3. 新增轮询函数
   - `KeyLedSelfTest_Poll()`
4. 新增输出统一下发函数
   - `KeyLedSelfTest_ApplyOutputs()`

## 4. 当前行为定义

当前逻辑不是“按住亮、松开灭”，而是：

- 上电默认灭灯
- 当 `PD5` 或 `PD6` 任一按键第一次被按下时
- 将 `PB9` 和 `PE0` 两灯同时点亮
- 亮灯状态锁存保持

这样做的原因是：

- 更容易肉眼确认主循环是否真正检测到了按键沿
- 更适合作为“主控是否活着”的最小验证

## 5. 当前不会影响的部分

本次没有拆掉原有主控框架，以下内容仍然保留：

- `F103 -> USART6` 短帧接收
- `GrayModule_Service()`
- `OLED` 灰度监视页刷新
- `MotorDrive_Init()` 默认安全停机

因此本次属于：

- 在现有主控框架之上叠加一个最小按键灯测试

而不是改写整套主循环。

## 6. 备份

本次修改前备份目录：

- `backups\2026-05-14_f407_key_led_selftest`

已备份：

- `main.c.bak`

## 7. 编译验证

已执行 `Keil` 工程批量构建，构建日志文件：

- [F407VET6_5.build_log.htm](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/MDK-ARM/F407VET6_5.1_Test/F407VET6_5.build_log.htm:1)

结果：

- `0 Error(s), 0 Warning(s)`

## 8. 上板预期

当前固件烧录后，最直接的实板预期是：

1. 上电后 `PB9 / PE0` 默认熄灭
2. 按下 `D5(PD5)` 一次，两个灯点亮并保持
3. 或按下 `D6(PD6)` 一次，两个灯点亮并保持

如果这个现象成立，至少说明当前主控侧以下基础链路是通的：

- 主循环在跑
- `PD5/PD6` 输入有效
- `PB9/PE0` 输出有效
- 程序确实已进入新固件逻辑

