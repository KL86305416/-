# F407 TB6612 Motor Layer Backup

- Date: `2026-05-16`
- Workspace: `D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- Scope: `F407 main control panel TB6612\code\5.15_F407_TB6612`
- Purpose: preserve the motor-related baseline before implementing the TB6612 motor control layer

## Backed-up files

- `code_before_edit\main.c`
- `code_before_edit\gpio.c`
- `code_before_edit\tim.c`
- `code_before_edit\5.15_F407_TB6612.uvprojx`
- `code_before_edit\5.15_F407_TB6612.ioc`

## What changed after this backup

- added `Core\Inc\motor_drive.h`
- added `Core\Src\motor_drive.c`
- integrated `MotorDrive_Init()` into `main.c`
- changed `DJ_BIN2 (PA15)` from analog to GPIO output
- tuned `TIM1` and `TIM2` to motor PWM parameters suitable for TB6612
- synced the Keil project and `.ioc` file to the new motor layer
