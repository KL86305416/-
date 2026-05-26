import csi
import display
import image
import time


DISPLAY_WIDTH = 128
DISPLAY_HEIGHT = 160


def build_test_image():
    img = image.Image(DISPLAY_WIDTH, DISPLAY_HEIGHT, csi.RGB565)
    half_w = DISPLAY_WIDTH // 2
    half_h = DISPLAY_HEIGHT // 2

    img.draw_rectangle((0, 0, half_w, half_h), color=(255, 0, 0), fill=True)
    img.draw_rectangle((half_w, 0, half_w, half_h), color=(0, 255, 0), fill=True)
    img.draw_rectangle((0, half_h, half_w, half_h), color=(0, 0, 255), fill=True)
    img.draw_rectangle((half_w, half_h, half_w, half_h), color=(255, 255, 255), fill=True)

    img.draw_rectangle((0, 0, DISPLAY_WIDTH, 44), color=(0, 0, 0), fill=True)
    img.draw_string(2, 2, "LCD MIN TEST", color=(255, 255, 255), scale=2)
    img.draw_string(2, 22, "official path", color=(255, 255, 0), scale=2)
    img.draw_cross(DISPLAY_WIDTH // 2, DISPLAY_HEIGHT // 2, color=(255, 255, 0), size=14, thickness=2)
    return img


def init_lcd():
    # Keep this path as close as possible to the official 1.8-inch LCD Shield
    # example so garbling can be attributed to hardware/firmware, not tuning.
    lcd = display.SPIDisplay(vflip=True, hmirror=True)
    lcd.backlight(1)
    return lcd


def main():
    lcd = init_lcd()
    img = build_test_image()

    while True:
        lcd.write(img)
        time.sleep_ms(80)


main()
