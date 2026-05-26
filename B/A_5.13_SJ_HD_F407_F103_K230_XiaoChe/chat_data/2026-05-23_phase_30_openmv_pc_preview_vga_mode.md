# Phase 30

- Date: `2026-05-23`
- Topic: `openmv_pc_preview_vga_mode`
- Clarification:
  - The user clarified that the official screen path is currently abnormal, and the real request is to improve the image resolution seen on the computer.
- Result:
  - Active `main.py` now defaults to a screenless preview-first mode.
  - Camera capture for IDE/USB preview is raised to `VGA`.
  - The red-circle recognition pipeline and `UART7 -> F407` link remain active.
  - Telemetry is still normalized to the previous `320x240` scale so the F407 side does not see doubled `DX/R` values.
