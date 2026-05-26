# OpenMV LCD minimal self-test

- Date: `2026-05-23`
- Status: `implemented`
- Scope:
  - `OpenMV Visual module(SJ)\code\main.py`
  - `OpenMV Visual module(SJ)\code\lcd_probe.py`
  - `OpenMV Visual module(SJ)\code\lcd_min_test.py`
  - `OpenMV Visual module(SJ)\code\README.md`
- Goal:
  - separate LCD bus/controller issues from camera/image-pipeline issues
- Change:
  - reduced the default LCD refresh to `20`
  - added `lcd_min_test.py` which does not use the camera and repeatedly displays solid-color quadrants plus text
  - kept `lcd_probe.py` for parameter combination probing, also at lower refresh
- Verification:
  - `python -m py_compile ...\main.py ...\lcd_probe.py ...\lcd_min_test.py`
  - passed
- Diagnostic interpretation:
  - if `lcd_min_test.py` still produces heavy garbling, the problem is very likely outside the main vision script logic
