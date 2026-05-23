# 项目日志：双舵机 PWM 调试控制

## 修改时间

2026-05-07

## 修改目标

在“舵机调试”功能中新增第二个舵机，并使用四个用户按键同时控制两个舵机左右移动，不再保留返回按钮。

## 修改内容

- 在 `Core/Src/gpio.c` 中新增 PA7 复用输出配置：
  - PA6：TIM3_CH1，舵机 1 PWM 输出。
  - PA7：TIM3_CH2，舵机 2 PWM 输出。
- 扩展 `Core/Inc/servo.h`、`Core/Src/servo.c`：
  - 新增 `Servo_Channel_t`，支持 `SERVO_CHANNEL_1` 和 `SERVO_CHANNEL_2`。
  - TIM3 同时输出 CH1、CH2 两路 50Hz PWM。
  - 新增双通道角度接口：
    - `Servo_SetChannelAngle()`
    - `Servo_SetChannelAngleX10()`
    - `Servo_CenterChannel()`
    - `Servo_GetChannelAngle()`
    - `Servo_GetChannelAngleX10()`
  - 原单舵机接口保留，默认控制舵机 1。
- 修改 `Core/Src/main.c` 中的舵机调试逻辑：
  - PC3 / PC2 控制舵机 1 增减角度。
  - PC1 / PC0 控制舵机 2 增减角度。
  - 舵机调试界面中 PC0 不再返回主菜单。
  - 进入舵机调试界面时，两个舵机都会回到中心角度。
  - 确认键进入舵机调试后，需要松开按键再控制舵机，避免 PC1 长按确认时误触发舵机 2。

## 当前按键说明

- PC3：舵机 1 角度增加。
- PC2：舵机 1 角度减少。
- PC1：舵机 2 角度增加。
- PC0：舵机 2 角度减少。

## 验证结果

- 已对 `Core/Src/main.c` 执行 `arm-none-eabi-gcc -fsyntax-only` 语法检查，通过。
- 已对 `Core/Src/servo.c` 执行 `arm-none-eabi-gcc -fsyntax-only` 语法检查，通过。
- 已对 `Core/Src/gpio.c` 执行 `arm-none-eabi-gcc -fsyntax-only` 语法检查，通过。

## Git 状态

当前目录不是 Git 仓库，`git status` 返回 `fatal: not a git repository`，因此本次无法创建 git 提交。
