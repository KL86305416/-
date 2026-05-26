# F407 DRV8874 Pan Invert Fix

- Date: `2026-05-26`
- Status: `implemented`
- Target project:
  - `F407 main control panel DRV8874\code\AAA_F407VET6_5.6_Test`
- Trigger:
  - After baud-rate and axis-UART mapping fixes, the left-right pan axis was confirmed to move in the opposite direction of the target.
- Main changes:
  - Flipped `GIMBAL_INVERT_PAN` from `0U` to `1U` in `gimbal_tracker.c`.
- Scope note:
  - Only the left-right axis direction was changed.
  - Up-down tilt direction was left unchanged.
- Verification:
  - Backup created at:
    - `backups/2026-05-26_f407_drv8874_pan_invert_fix`
- Runtime expectation:
  - `x_offset` tracking should now drive the left-right axis in the corrected direction.
