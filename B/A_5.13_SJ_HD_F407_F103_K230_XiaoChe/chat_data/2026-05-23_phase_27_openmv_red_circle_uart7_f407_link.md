# Phase 27

- Date: `2026-05-23`
- Topic: `openmv_red_circle_uart7_f407_link`
- Result:
  - OpenMV N6 now detects a red circle-like target, computes horizontal offset, and sends it over `UART7`.
  - F407 now receives the vision frame on `UART4`, parses it, and exposes the result on the OLED diagnostics page.
- Protocol:
  - frame length `9`
  - `SOF = A5 5A`
  - payload = `x_offset(i16 LE) + radius(u16 LE) + flags(u8) + seq(u8)`
  - `CRC8` over payload bytes
