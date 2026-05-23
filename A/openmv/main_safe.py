import sensor
import time

try:
    from machine import UART
except ImportError:
    from pyb import UART


RED_THRESHOLD = (30, 100, 15, 127, 15, 127)
UART_PORT = 3
UART_BAUDRATE = 115200
MIN_AREA = 80


sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=1200)
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)

uart = UART(UART_PORT, UART_BAUDRATE, timeout_char=20)
clock = time.clock()


def best_blob(blobs):
    best = None
    best_area = 0
    for blob in blobs or []:
        try:
            area = blob.pixels()
        except Exception:
            area = blob.w() * blob.h()
        if area > best_area:
            best = blob
            best_area = area
    return best


while True:
    clock.tick()
    img = sensor.snapshot()
    try:
        blobs = img.find_blobs([RED_THRESHOLD], pixels_threshold=MIN_AREA,
                               area_threshold=MIN_AREA, merge=True)
        target = best_blob(blobs)
        if target:
            img.draw_rectangle(target.rect())
            img.draw_cross(target.cx(), target.cy())
            uart.write("1,%d,%d\n" % (target.cx(), target.cy()))
        else:
            uart.write("0,0,0\n")
    except Exception:
        uart.write("0,0,0\n")
    time.sleep_ms(20)
