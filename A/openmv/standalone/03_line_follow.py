import sensor
import time

try:
    from machine import UART
except ImportError:
    from pyb import UART


BLACK_THRESHOLD = (0, 35, -30, 30, -30, 30)
UART_PORT = 3
UART_BAUDRATE = 115200
MIN_PIXELS = 100
MIN_AREA = 100
DRAW = True

# x, y, w, h, weight
ROIS = (
    (0, 160, 320, 40, 0.2),
    (0, 190, 320, 30, 0.3),
    (0, 220, 320, 20, 0.5),
)


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
    weighted_x = 0
    weight_sum = 0
    hit_count = 0

    for roi in ROIS:
        rect = roi[:4]
        weight = roi[4]
        blobs = img.find_blobs([BLACK_THRESHOLD], roi=rect,
                               pixels_threshold=MIN_PIXELS,
                               area_threshold=MIN_AREA,
                               merge=True)
        target = best_blob(blobs)
        if target:
            hit_count += 1
            weighted_x += target.cx() * weight
            weight_sum += weight
            if DRAW:
                img.draw_rectangle(target.rect())
                img.draw_cross(target.cx(), target.cy())
        elif DRAW:
            img.draw_rectangle(rect)

    if hit_count and weight_sum:
        cx = int(weighted_x / weight_sum)
        error = cx - 160
        if DRAW:
            img.draw_line(160, 240, cx, 180)
        uart.write("1,%d,%d,%d\n" % (cx, error, hit_count))
        print("line cx=%d error=%d hit=%d fps=%d" %
              (cx, error, hit_count, int(clock.fps())))
    else:
        uart.write("0,0,0,0\n")
        print("line none fps=%d" % int(clock.fps()))
    time.sleep_ms(20)
