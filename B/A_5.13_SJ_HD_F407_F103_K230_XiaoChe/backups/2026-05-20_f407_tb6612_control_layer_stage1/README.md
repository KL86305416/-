# Backup: F407 TB6612 Control Layer Stage1
- Date: `2026-05-20`
- Workspace: `D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- Purpose: backup the mainline F407/TB6612 control entry point before adding the key-based motor self-test layer and before cleaning the motor driver warning.

## Files
- `main.c.bak`
- `motor_drive.c.bak`

## Baseline Description
- USART6 gray short-frame receive was already working.
- OLED gray monitor was already working.
- TB6612 motor layer was already initialized, but the main loop still had no practical motor control logic.
- This backup preserves the stable state just before the first mainline control-layer integration.
