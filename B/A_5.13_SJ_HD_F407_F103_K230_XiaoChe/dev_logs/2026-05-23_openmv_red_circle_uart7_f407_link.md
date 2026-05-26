# OpenMV UART7 vision link

- Date: `2026-05-23`
- Status: `implemented`
- OpenMV side:
  - `red circle-like target detection`
  - `UART7 (P14/P13) @ 115200`
  - `9-byte frame: SOF A5 5A + x_offset(i16) + radius(u16) + flags + seq + crc8`
  - `display overlay retained`
- F407 side:
  - `UART4 interrupt receive enabled`
  - `frame parser added in main.c`
  - `OLED diagnostics updated with vision link state`
- Verification:
  - `python -m py_compile ...\OpenMV Visual module(SJ)\code\main.py`
  - `UV4.exe -b ...\5.15_F407_TB6612.uvprojx -j0`
  - both returned success
