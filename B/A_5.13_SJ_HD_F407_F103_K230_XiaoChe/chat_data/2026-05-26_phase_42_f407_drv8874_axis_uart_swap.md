# Phase 42

- Date: `2026-05-26`
- Topic: `f407_drv8874_axis_uart_swap`
- Trigger:
  - The user confirmed the actual rig mapping:
    - `UART5` handles up/down
    - `USART6` handles left/right
- Result:
  - The DRV8874 F407 firmware axis binding was updated accordingly.
  - The software now maps:
    - `PAN / x_offset / left-right` -> `USART6`
    - `TILT / y_offset / up-down` -> `UART5`
