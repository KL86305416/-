import csi
import display
import image
import time


csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize((128, 160))

lcd = display.SPIDisplay(vflip=True, hmirror=True)
lcd.backlight(1)
clock = time.clock()

while True:
    clock.tick()
    lcd.write(csi0.snapshot(), hint=image.CENTER | image.SCALE_ASPECT_KEEP)
    print(clock.fps())
