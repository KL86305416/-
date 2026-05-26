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
3. If you want the official LCD active, keep `ENABLE_LCD = True`.
4. If the LCD is heavily garbled, run `lcd_shield_exact.py` first.
5. If `lcd_shield_exact.py` still garbles, run `lcd_min_test.py` to isolate the LCD path from the camera path.
6. If the LCD shows abnormal colors/orientation but is still basically readable, run `lcd_probe.py` and note which mode looks correct.
7. Tune these LCD parameters in `main.py` if needed:
   - `LCD_BGR`
   - `LCD_BYTE_SWAP`
   - `LCD_HMIRROR`
   - `LCD_VFLIP`
   - `LCD_REFRESH`
8. Wire `UART7` to the F407 board:
   - `OpenMV P14 (UART7 TX)` -> `F407 OpenMV-TX / UART4_RX`
   - `OpenMV P13 (UART7 RX)` -> `F407 OpenMV-RX / UART4_TX`
   - `GND` -> `GND`
9. Upload `main.py` to the OpenMV N6 board.
10. Power-cycle the board.

## Official examples synced locally

- Official OpenMV examples are now available under `examples/openmv_official`.
- The synced source is the official `openmv/openmv` repository `scripts/examples` tree.
- See `examples/README.md` for the source commit and the most relevant N6/display example paths.

## Notes

- The script uses the official OpenMV `csi` and `display.SPIDisplay` APIs.
- The current runtime-compatible path does not import `machine.PWM`.
- Backlight is initialized through the display's default GPIO controller, so `BACKLIGHT_PERCENT > 0` acts as `on`.
- The current behavior is `black-bordered rectangle detection + UART7 output + on-screen overlay`.
- Vision frame is 11 bytes: `A5 5A x_lo x_hi y_lo y_hi w_lo w_hi flags seq crc8` (CRC8 poly 0x07 over bytes 2..9, x/y are int16 LE offsets from image center, w is uint16 LE bounding-box width, both mapped to the telemetry size of the active screen).
- `flags` bits: `0x01 VALID`, `0x02 RECT_FOUND`, `0x04 RECT_GOOD`.
- The image is scaled to fit the display while keeping aspect ratio.
- FPS is printed to the REPL/serial console by default.
- `main.py` now uses the same minimal `SPIDisplay(vflip=True, hmirror=True)` init path as the official 1.8-inch LCD Shield example.
- `lcd_shield_exact.py` is the closest possible sanity check against the official 1.8-inch LCD Shield example.
- `lcd_min_test.py` is a camera-independent self-test for a heavily garbled official screen.
- `lcd_probe.py` is a second-stage LCD parameter probe for abnormal but still readable screen output.
