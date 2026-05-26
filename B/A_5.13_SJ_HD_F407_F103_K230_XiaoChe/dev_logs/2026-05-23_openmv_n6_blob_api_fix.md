# OpenMV N6 blob API compatibility fix

- Date: `2026-05-23`
- Status: `implemented`
- Scope:
  - `OpenMV Visual module(SJ)\code\main.py`
- Problem:
  - Runtime raised `TypeError: unsupported types for __gt__: 'bound_method', 'int'` inside `classify_red_circle()`.
  - On this N6 firmware build, blob members such as `w`, `h`, `cx`, `cy`, `rect`, `roundness`, and `density` are exposed as callable accessors.
- Fix:
  - added a compatibility helper that reads a member as either `obj.member` or `obj.member()`
  - converted red-circle classification and overlay drawing to use the compatibility helper everywhere
- Verification:
  - `python -m py_compile ...\OpenMV Visual module(SJ)\code\main.py`
  - no remaining direct `blob.w / blob.h / blob.cx / blob.rect / blob.roundness / blob.density` access in active script
- Hardware next step:
  - upload the updated `main.py`
  - verify live image, FPS overlay, red-target overlay, and `UART7` link to F407
