import sensor
import time

try:
    from machine import UART
except ImportError:
    from pyb import UART


UART_PORT = 3
UART_BAUDRATE = 115200
ROI = (0, 0, 320, 240)
DRAW = True


def best_circle(circles):
    best = None
    best_score = 0
    for c in circles:
        score = c.magnitude()
        if score > best_score:
            best = c
            best_score = score
    return best


def best_rect(rects):
    best = None
    best_score = 0
    for r in rects:
        score = r.magnitude()
        if score > best_score:
            best = r
            best_score = score
    return best


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
    circles = img.find_circles(roi=ROI, threshold=2500,
                               x_margin=10, y_margin=10, r_margin=10,
                               r_min=5, r_max=80, r_step=2)
    circle = best_circle(circles)
    if circle:
        x = circle.x()
        y = circle.y()
        r = circle.r()
        if DRAW:
            img.draw_circle(x, y, r)
            img.draw_cross(x, y)
        uart.write("C,%d,%d,%d,%d\n" % (x, y, r, circle.magnitude()))
        print("circle x=%d y=%d r=%d fps=%d" %
              (x, y, r, int(clock.fps())))
        time.sleep_ms(20)
        continue

    rects = img.find_rects(roi=ROI, threshold=10000)
    rect = best_rect(rects)
    if rect:
        x = rect.x() + rect.w() // 2
        y = rect.y() + rect.h() // 2
        if DRAW:
            img.draw_rectangle(rect.rect())
            img.draw_cross(x, y)
        uart.write("R,%d,%d,%d,%d,%d\n" %
                   (x, y, rect.w(), rect.h(), rect.magnitude()))
        print("rect x=%d y=%d fps=%d" % (x, y, int(clock.fps())))
    else:
        uart.write("N,0,0,0,0,0\n")
        print("shape none fps=%d" % int(clock.fps()))
    time.sleep_ms(20)
