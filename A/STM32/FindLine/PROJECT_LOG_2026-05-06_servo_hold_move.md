# 项目日志：舵机调试长按连续移动

## 修改时间

2026-05-06

## 修改目标

将“舵机调试”菜单中的 PC3、PC2 控制逻辑从单次按下移动一次，改为按住按钮时舵机持续改变角度。

## 修改内容

- 在 `Core/Src/main.c` 中新增 `APP_SERVO_REPEAT_MS`，用于统一控制舵机长按重复移动间隔。
- 新增 `servo_last_move_tick`，记录上一次舵机移动时间。
- 新增 `App_ButtonPressedMask()`，读取消抖后的按键持续按住状态。
- 修改 `App_UpdateServoDebug()`：
  - PC3 按下立即增加角度，长按持续增加。
  - PC2 按下立即减少角度，长按持续减少。
  - PC1 仍然用于舵机回中。
  - PC3 和 PC2 同时按住时不移动，避免方向冲突。

## 当前按键说明

- PC3：舵机角度增加，支持长按连续移动。
- PC2：舵机角度减少，支持长按连续移动。
- PC1：舵机回中。
- PC0：返回主菜单。

## 验证结果

- 已对 `Core/Src/main.c` 执行 `arm-none-eabi-gcc -fsyntax-only` 语法检查，通过。
- 已对 `Core/Src/servo.c` 执行 `arm-none-eabi-gcc -fsyntax-only` 语法检查，通过。
- 已对 `Core/Src/gpio.c` 执行 `arm-none-eabi-gcc -fsyntax-only` 语法检查，通过。

## Git 状态

当前目录不是 Git 仓库，`git status` 返回 `fatal: not a git repository`，因此本次无法创建 git 提交。
