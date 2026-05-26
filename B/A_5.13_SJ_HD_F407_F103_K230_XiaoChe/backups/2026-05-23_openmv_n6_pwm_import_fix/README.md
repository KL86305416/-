# OpenMV N6 Vision Main

This folder contains the active `main.py` entry script for the OpenMV N6.

Supported screens:

- `lcd_1p8`
  - OpenMV LCD Shield
  - `128x160`
  - default backlight pin `P5`

- `touch_lcd_2p3`
  - OpenMV Touch LCD Shield
  - `320x240`
  - default backlight pin `P6`

## How to use

1. Open [main.py](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/OpenMV%20Visual%20module(SJ)/code/main.py:1)
2. Set `DISPLAY_KIND` to match your screen.
3. Wire `UART7` to the F407 board:
   - `OpenMV P14 (UART7 TX)` -> `F407 OpenMV-TX / UART4_RX`
   - `OpenMV P13 (UART7 RX)` -> `F407 OpenMV-RX / UART4_TX`
   - `GND` -> `GND`
4. Upload `main.py` to the OpenMV N6 board.
5. Power-cycle the board.

## Official examples synced locally

- Official OpenMV examples are now available under `examples/openmv_official`.
- The synced source is the official `openmv/openmv` repository `scripts/examples` tree.
- See `examples/README.md` for the source commit and the most relevant N6/display example paths.

## Notes

- The script uses the official OpenMV `csi` and `display.SPIDisplay` APIs.
- The current runtime-compatible path does not import `machine.PWM`.
- Backlight is initialized through the display's default GPIO controller, so `BACKLIGHT_PERCENT > 0` acts as `on`.
- The current behavior is `red blob / circle-like target detection + UART7 output + on-screen overlay`.
- The image is scaled to fit the display while keeping aspect ratio.
- FPS is printed to the REPL/serial console by default.
