# Phase 32

- Date: `2026-05-23`
- Topic: `openmv_lcd_probe_and_restore`
- Result:
  - The main OpenMV vision script now enables the official LCD path again.
  - LCD controller flags were surfaced as editable constants instead of staying hard-coded.
  - A separate `lcd_probe.py` was added to empirically determine the correct display parameter set when the official screen output is abnormal.
