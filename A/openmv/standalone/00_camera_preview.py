import sensor
import time


PIXFORMAT = sensor.RGB565
FRAMESIZE = sensor.QVGA
LOCK_AUTO = True
EXPOSURE_US = 8000
GAIN_DB = 8


sensor.reset()
sensor.set_pixformat(PIXFORMAT)
sensor.set_framesize(FRAMESIZE)
sensor.skip_frames(time=1000)

if LOCK_AUTO:
    sensor.set_auto_gain(False, gain_db=GAIN_DB)
    sensor.set_auto_whitebal(False)
    sensor.set_auto_exposure(False, exposure_us=EXPOSURE_US)

clock = time.clock()

while True:
    clock.tick()
    img = sensor.snapshot()
    print("fps=%d" % int(clock.fps()))
