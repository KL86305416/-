# F407 按键单灯实时自检更新
- 日期：2026-05-14
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`implemented and build-verified`
- 范围：`F407 main control panel\code\AAA_F407VET6_5.6_Test`

## 1. 本次变更目的

上一版自检逻辑是：

- 任一按键按下一次
- 两个灯同时点亮
- 并保持锁存

本次按用户要求改为更直接的板级输入输出验证：

- 一个按键对应一个灯
- 按下就亮
- 抬起就灭

## 2. 当前映射关系

按本次对话中的引脚顺序采用一对一映射：

- `D5 -> PD5 -> KEY_3 -> B9 -> PB9 -> LED_1`
- `D6 -> PD6 -> KEY_4 -> E0 -> PE0 -> LED_2`

说明：

- `PD5 / PD6` 为硬件上拉、按下接地
- 因此程序按“低电平按下”处理
- `PB9 / PE0` 为高电平点亮

## 3. 代码改动

修改文件：

- [main.c](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/Core/Src/main.c:1)

本次将自检逻辑从“锁存式状态机”改成“实时电平跟随”：

1. 删除原来的锁存状态变量
   - `g_key_led_latched`
   - `g_key_any_prev_pressed`
2. 将 `KeyLedSelfTest_ApplyOutputs()` 改为直接读取按键并下发灯输出
3. `KeyLedSelfTest_Poll()` 不再做边沿检测，只做实时刷新

## 4. 当前行为定义

烧录当前固件后的预期表现为：

- 上电默认两灯熄灭
- 按住 `D5(PD5)` 时，`B9(PB9)` 亮
- 松开 `D5(PD5)` 时，`B9(PB9)` 灭
- 按住 `D6(PD6)` 时，`E0(PE0)` 亮
- 松开 `D6(PD6)` 时，`E0(PE0)` 灭

## 5. 备份

本次修改前备份目录：

- `backups\2026-05-14_f407_key_led_realtime`

已备份：

- `main.c.bak`

## 6. 编译验证

已重新执行 `Keil` 批量构建，构建输出文件：

- [F407VET6_5.build_log.htm](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/MDK-ARM/F407VET6_5.1_Test/F407VET6_5.build_log.htm:1)

结果：

- `0 Error(s), 0 Warning(s)`

