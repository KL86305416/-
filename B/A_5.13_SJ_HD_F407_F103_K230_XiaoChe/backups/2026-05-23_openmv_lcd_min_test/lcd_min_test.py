import csi
import display
import image
import time


DISPLAY_WIDTH = 320
DISPLAY_HEIGHT = 240
LCD_REFRESH = 20
MODE_HOLD_MS = 2500

CONFIGS = (
    {"name": "official", "bgr": True, "byte_swap": False, "hmirror": False, "vflip": False},
    {"name": "swap_only", "bgr": True, "byte_swap": True, "hmirror": False, "vflip": False},
    {"name": "rgb_only", "bgr": False, "byte_swap": False, "hmirror": False, "vflip": False},
    {"name": "rot180", "bgr": True, "byte_swap": False, "hmirror": True, "vflip": True},
)


def build_test_image(cfg):
    img = image.Image(DISPLAY_WIDTH, DISPLAY_HEIGHT, csi.RGB565)
    half_w = DISPLAY_WIDTH // 2
    half_h = DISPLAY_HEIGHT // 2

    img.draw_rectangle((0, 0, half_w, half_h), color=(255, 0, 0), fill=True)
    img.draw_rectangle((half_w, 0, half_w, half_h), color=(0, 255, 0), fill=True)
    img.draw_rectangle((0, half_h, half_w, half_h), color=(0, 0, 255), fill=True)
    img.draw_rectangle((half_w, half_h, half_w, half_h), color=(255, 255, 255), fill=True)

    img.draw_rectangle((0, 0, DISPLAY_WIDTH, 44), color=(0, 0, 0), fill=True)
    img.draw_string(2, 2, "LCD MIN TEST", color=(255, 255, 255), scale=2)
    img.draw_string(2, 22, cfg["name"], color=(255, 255, 0), scale=2)
    img.draw_cross(DISPLAY_WIDTH // 2, DISPLAY_HEIGHT // 2, color=(255, 255, 0), size=14, thickness=2)
    return img


def init_lcd(cfg):
    lcd = display.SPIDisplay(
        width=DISPLAY_WIDTH,
        height=DISPLAY_HEIGHT,
        refresh=LCD_REFRESH,
        bgr=cfg["bgr"],
        byte_swap=cfg["byte_swap"],
        hmirror=cfg["hmirror"],
        vflip=cfg["vflip"],
    )
    lcd.backlight(1)
    return lcd


def main():
    while True:
        for cfg in CONFIGS:
            print("lcd_min_cfg:", cfg)
            lcd = init_lcd(cfg)
            img = build_test_image(cfg)
            start = time.ticks_ms()

            while time.ticks_diff(time.ticks_ms(), start) < MODE_HOLD_MS:
                lcd.write(img)
                time.sleep_ms(80)


main()
