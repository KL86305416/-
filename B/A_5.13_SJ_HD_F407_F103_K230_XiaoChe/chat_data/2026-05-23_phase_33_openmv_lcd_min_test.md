# Phase 33

- Date: `2026-05-23`
- Topic: `openmv_lcd_min_test`
- Trigger:
  - The official LCD still showed heavy garbling after re-enabling the display path and adding a parameter probe.
- Result:
  - A new camera-independent `lcd_min_test.py` was added to isolate the LCD pipeline itself.
  - LCD refresh was reduced to a more conservative value in both the main script and the LCD probe script.
