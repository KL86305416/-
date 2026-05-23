import sensor
import time

try:
    from machine import UART
except ImportError:
    from pyb import UART


TARGET_COLOR = "red"  # red / green / blue

THRESHOLDS = {
    "red": (30, 100, 15, 127, 15, 127),
    "green": (25, 100, -70, -10, 5, 80),
    "blue": (20, 90, -20, 40, -90, -20),
}

COLOR_ID = {
    "red": 1,
    "green": 2,
    "blue": 3,
}

ROI = (0, 0, 320, 240)
MIN_PIXELS = 80
MIN_AREA = 80
UART_PORT = 3
UART_BAUDRATE = 115200
DRAW = True


def best_blob(blobs):
    best = None
    best_pixels = 0
    for blob in blobs:
        pixels = blob.pixels()
        if pixels > best_pixels:
            best = blob
            best_pixels = pixels
    return best


sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=1000)
sensor.set_auto_gain(False, gain_db=8)
sensor.set_auto_whitebal(False)
sensor.set_auto_exposure(False, exposure_us=8000)

uart = UART(UART_PORT, UART_BAUDRATE, timeout_char=20)
clock = time.clock()
threshold = THRESHOLDS[TARGET_COLOR]
target_id = COLOR_ID[TARGET_COLOR]

while True:
    clock.tick()
    img = sensor.snapshot()
    blobs = img.find_blobs([threshold], roi=ROI,
                           pixels_threshold=MIN_PIXELS,
                           area_threshold=MIN_AREA,
                           merge=True)
    target = best_blob(blobs)
    if target:
        x = target.cx()
        y = target.cy()
        if DRAW:
            img.draw_rectangle(target.rect())
            img.draw_cross(x, y)
        uart.write("%d,%d,%d,%d,%d,%d\n" %
                   (target_id, x, y, target.w(), target.h(), target.pixels()))
        print("color=%s x=%d y=%d fps=%d" %
              (TARGET_COLOR, x, y, int(clock.fps())))
    else:
        uart.write("0,0,0,0,0,0\n")
        print("color=%s none fps=%d" % (TARGET_COLOR, int(clock.fps())))
    time.sleep_ms(20)
