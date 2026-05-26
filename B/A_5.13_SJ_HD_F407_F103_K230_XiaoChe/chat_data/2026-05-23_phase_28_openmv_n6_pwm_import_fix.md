# Phase 28

- Date: `2026-05-23`
- Topic: `openmv_n6_pwm_import_fix`
- Trigger:
  - The deployed OpenMV script failed at startup with `ImportError: can't import name PWM`.
- Result:
  - The runtime dependency on `machine.PWM` was removed from the active `main.py`.
  - Display startup now follows the official `display.SPIDisplay(...)` path so the script can run on the user's actual N6 firmware build.
  - The red-circle detection, FPS overlay, and `UART7` frame transmission logic were kept intact.
- Current next step:
  - Re-run `main.py` on the OpenMV N6 and verify live display plus UART link behavior on real hardware.
