import sensor
import time
import math

try:
    from machine import UART
except ImportError:
    from pyb import UART


UART_PORT = 3
UART_BAUDRATE = 115200
DRAW = True


sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=1000)
sensor.set_auto_gain(False)
sensor.set_auto_exposure(False, exposure_us=8000)

uart = UART(UART_PORT, UART_BAUDRATE, timeout_char=20)
clock = time.clock()

while True:
    clock.tick()
    img = sensor.snapshot()
    tags = img.find_apriltags()
    if tags:
        tag = tags[0]
        x = tag.cx()
        y = tag.cy()
        angle = int(tag.rotation() * 180 / math.pi)
        if DRAW:
            img.draw_rectangle(tag.rect())
            img.draw_cross(x, y)
            img.draw_string(x + 4, y + 4, str(tag.id()))
        uart.write("1,%d,%d,%d,%d\n" % (tag.id(), x, y, angle))
        print("tag id=%d x=%d y=%d angle=%d fps=%d" %
              (tag.id(), x, y, angle, int(clock.fps())))
    else:
        uart.write("0,0,0,0,0\n")
        print("tag none fps=%d" % int(clock.fps()))
    time.sleep_ms(20)
