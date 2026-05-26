# Phase 34

- Date: `2026-05-23`
- Topic: `openmv_lcd_official_exact_path`
- Trigger:
  - The LCD still showed direct garbling after switching the display type assumption to the 1.8-inch shield.
- Result:
  - The 1.8-inch LCD path in `main.py` was reduced to the same minimal init style as the official example.
  - A new `lcd_shield_exact.py` script was added as the closest possible official sanity check.
  - `lcd_min_test.py` was simplified to a camera-independent official-path test.
  - `lcd_probe.py` was kept as a second-stage tool for readable-but-wrong output instead of first-line recovery.
