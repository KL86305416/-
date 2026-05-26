# Backup: F407 TB6612 OLED Motor Diagnostic
- Date: `2026-05-21`
- Workspace: `D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- Purpose: back up the stage-1 key motor self-test baseline before exposing live motor-drive diagnostics on the OLED monitor.

## Files
- `main.c.bak`
- `gray_monitor.h.bak`
- `gray_monitor.c.bak`

## Baseline Description
- The mainline F407/TB6612 project had already entered the key-based motor self-test stage.
- `KEY_1 / KEY_2` were already driving `motor A` forward and reverse.
- `KEY_3 / KEY_4` were already driving `motor B` forward and reverse.
- `GrayMonitor_Poll()` was still showing only F103 gray runtime information.
- The vehicle-control self-test state was still private to `main.c` and had not yet been shared with the OLED monitor.

## Why This Backup Exists
- The next debugging question was no longer just "does the motor move".
- We also needed to see, on the board itself:
- whether the key press was detected,
- whether the requested percent was applied,
- whether the PWM compare value changed,
- and whether `AIN1/AIN2/BIN1/BIN2` matched the expected direction state.

This backup preserves the stable point just before that OLED-side drive diagnostic layer was added.
