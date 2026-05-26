# OpenMV PC preview VGA mode

- Date: `2026-05-23`
- Status: `implemented`
- Scope:
  - `OpenMV Visual module(SJ)\code\main.py`
- Goal:
  - prioritize higher-resolution IDE/USB preview on the computer
  - stop depending on the malfunctioning local LCD during current bring-up
- Change:
  - set `PREVIEW_SENSOR_FRAMESIZE = csi.VGA`
  - set `ENABLE_LCD = False` by default
  - keep red-circle detection and `UART7` output active
  - keep transmitted `DX/R` normalized to the existing `320x240` telemetry scale
- Verification:
  - `python -m py_compile ...\OpenMV Visual module(SJ)\code\main.py`
  - passed
- Notes:
  - the touch LCD shield itself is still a separate hardware/display-path issue
  - if preview FPS is too low, revert preview resolution from `csi.VGA` to `csi.QVGA`
