# 项目日志：舵机调试快速匀速移动优化

## 修改时间

2026-05-06

## 修改目标

优化“舵机调试”菜单中的按键控制手感，让 PC3、PC2 长按时舵机更快、更均匀地改变角度，同时减少 OLED 实时刷新带来的算力占用。

## 修改内容

- 在 `Core/Src/main.c` 中新增 `APP_SERVO_CONTROL_PERIOD_MS`，舵机调试界面使用更短的循环延时，提高按键长按控制刷新频率。
- 在 `Core/Src/main.c` 中新增 `APP_SERVO_SPEED_DEG_X10_PER_SEC`，统一控制舵机长按移动速度，当前为每秒 300.0 度。
- 将舵机长按移动逻辑改为基于 `HAL_GetTick()` 的时间差计算，使角度按固定速度推进，而不是按固定大步长一段一段跳变。
- 取消舵机移动过程中的 OLED 实时角度刷新，只在进入舵机调试界面和 PC1 回中时刷新界面。
- 在 `Core/Inc/servo.h`、`Core/Src/servo.c` 中新增 0.1 度单位的舵机角度接口：
  - `Servo_SetAngleX10()`
  - `Servo_GetAngleX10()`
- 原有整数角度接口仍然保留，避免影响其他模块调用。

## 当前参数

- 舵机调试循环延时：`APP_SERVO_CONTROL_PERIOD_MS = 2U`
- 舵机长按移动速度：`APP_SERVO_SPEED_DEG_X10_PER_SEC = 3000U`
- 角度单位：0.1 度

## 验证结果

- 已对 `Core/Src/main.c` 执行 `arm-none-eabi-gcc -fsyntax-only` 语法检查，通过。
- 已对 `Core/Src/servo.c` 执行 `arm-none-eabi-gcc -fsyntax-only` 语法检查，通过。

## Git 状态

当前目录不是 Git 仓库，`git status` 返回 `fatal: not a git repository`，因此本次无法创建 git 提交。
