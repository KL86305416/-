import csi
import display
import image
import time


DISPLAY_WIDTH = 320
DISPLAY_HEIGHT = 240
SETTLE_MS = 1200
FRAMES_PER_MODE = 90

CONFIGS = (
    {"name": "official", "bgr": True, "byte_swap": False, "hmirror": False, "vflip": False},
    {"name": "swap_only", "bgr": True, "byte_swap": True, "hmirror": False, "vflip": False},
    {"name": "rgb_only", "bgr": False, "byte_swap": False, "hmirror": False, "vflip": False},
    {"name": "rgb_swap", "bgr": False, "byte_swap": True, "hmirror": False, "vflip": False},
    {"name": "rot180", "bgr": True, "byte_swap": False, "hmirror": True, "vflip": True},
    {"name": "rot180_swap", "bgr": True, "byte_swap": True, "hmirror": True, "vflip": True},
    {"name": "rot180_rgb", "bgr": False, "byte_swap": False, "hmirror": True, "vflip": True},
    {"name": "rot180_all", "bgr": False, "byte_swap": True, "hmirror": True, "vflip": True},
)


def init_camera():
    cam = csi.CSI()
    cam.reset()
    cam.pixformat(csi.RGB565)
    cam.framesize(csi.QVGA)
    cam.colorbar(True)
    cam.snapshot(time=1500)
    return cam


def init_lcd(cfg):
    lcd = display.SPIDisplay(
        width=DISPLAY_WIDTH,
        height=DISPLAY_HEIGHT,
        refresh=60,
        bgr=cfg["bgr"],
        byte_swap=cfg["byte_swap"],
        hmirror=cfg["hmirror"],
        vflip=cfg["vflip"],
    )
    lcd.backlight(1)
    return lcd


def draw_probe_frame(frame, cfg, mode_index):
    frame.draw_rectangle((0, 0, frame.width(), 44), color=(0, 0, 0), fill=True)
    frame.draw_string(2, 2, "LCD PROBE %d/%d" % (mode_index + 1, len(CONFIGS)), color=(255, 255, 255), scale=2)
    frame.draw_string(2, 22, cfg["name"], color=(255, 255, 0), scale=2)
    frame.draw_string(
        2,
        42,
        "bgr=%d bs=%d hm=%d vf=%d" % (
            1 if cfg["bgr"] else 0,
            1 if cfg["byte_swap"] else 0,
            1 if cfg["hmirror"] else 0,
            1 if cfg["vflip"] else 0,
        ),
        color=(255, 255, 255),
        scale=1,
    )
    frame.draw_cross(frame.width() // 2, frame.height() // 2, color=(255, 255, 255), size=12, thickness=2)


def main():
    cam = init_camera()
    clock = time.clock()

    while True:
        for index, cfg in enumerate(CONFIGS):
            print("lcd_cfg:", index, cfg)
            lcd = init_lcd(cfg)
            start = time.ticks_ms()

            while time.ticks_diff(time.ticks_ms(), start) < SETTLE_MS:
                frame = cam.snapshot()
                draw_probe_frame(frame, cfg, index)
                lcd.write(frame, hint=image.CENTER | image.SCALE_ASPECT_KEEP)

            for _ in range(FRAMES_PER_MODE):
                clock.tick()
                frame = cam.snapshot()
                draw_probe_frame(frame, cfg, index)
                lcd.write(frame, hint=image.CENTER | image.SCALE_ASPECT_KEEP)
                print("fps:", clock.fps(), "mode:", cfg["name"])


main()
