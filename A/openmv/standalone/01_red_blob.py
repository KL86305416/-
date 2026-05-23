import sensor
import time

try:
    from machine import UART
except ImportError:
    from pyb import UART


RED_THRESHOLD = (30, 100, 15, 127, 15, 127)
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

while True:
    clock.tick()
    img = sensor.snapshot()
    blobs = img.find_blobs([RED_THRESHOLD], roi=ROI,
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
        uart.write("1,%d,%d,%d,%d,%d\n" %
                   (x, y, target.w(), target.h(), target.pixels()))
        print("ok=1 x=%d y=%d fps=%d" % (x, y, int(clock.fps())))
    else:
        uart.write("0,0,0,0,0,0\n")
        print("ok=0 fps=%d" % int(clock.fps()))
    time.sleep_ms(20)
