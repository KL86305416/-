import sensor
import time

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
    codes = img.find_qrcodes()
    if codes:
        code = codes[0]
        x = code.x() + code.w() // 2
        y = code.y() + code.h() // 2
        payload = code.payload()
        if DRAW:
            img.draw_rectangle(code.rect())
            img.draw_cross(x, y)
            img.draw_string(code.x(), code.y(), payload)
        uart.write("1,%d,%d,%s\n" % (x, y, payload))
        print("qr x=%d y=%d text=%s fps=%d" %
              (x, y, payload, int(clock.fps())))
    else:
        uart.write("0,0,0,\n")
        print("qr none fps=%d" % int(clock.fps()))
    time.sleep_ms(20)
