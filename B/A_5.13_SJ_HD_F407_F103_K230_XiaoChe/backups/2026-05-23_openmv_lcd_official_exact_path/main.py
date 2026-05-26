import csi
import display
import image
import time
from machine import UART


# Set this to match the official screen you actually attached.
# Supported values:
# - "lcd_1p8"     -> OpenMV LCD Shield, 128x160, backlight on P5
# - "touch_lcd_2p3" -> OpenMV Touch LCD Shield, 320x240, backlight on P6
DISPLAY_KIND = "lcd_1p8"
ENABLE_LCD = True
PREVIEW_SENSOR_FRAMESIZE = csi.VGA
LCD_REFRESH = 20
LCD_BGR = False
LCD_BYTE_SWAP = False
LCD_HMIRROR = True
LCD_VFLIP = True

BACKLIGHT_PERCENT = 80
PRINT_FPS_TO_REPL = True
PRINT_VISION_TO_REPL = False

# Touch LCD Shield uses P2/P3/P4/P5/P6, so UART7 (P14/P13) avoids conflicts.
UART_PORT = 7
UART_BAUD = 115200

VISION_SOF1 = 0xA5
VISION_SOF2 = 0x5A
VISION_FLAG_VALID = 0x01
VISION_FLAG_RED_FOUND = 0x02
VISION_FLAG_CIRCLE_LIKE = 0x04

RED_THRESHOLDS = [
    (30, 100, 15, 127, 15, 127),
]
PIXELS_THRESHOLD = 250
AREA_THRESHOLD = 250
MERGE_MARGIN = 10
MIN_ASPECT_RATIO = 0.60
MIN_ROUNDNESS = 0.35
MIN_DENSITY = 0.22
MIN_COMPACTNESS = 0.18
MIN_DISC_FILL = 0.55
MIN_RED_PIXELS = 350


def build_display_config(kind):
    if kind == "lcd_1p8":
        return {
            "sensor_framesize": (128, 160),
            "telemetry_size": (128, 160),
            "display_hint": image.CENTER | image.SCALE_ASPECT_KEEP,
            "display_kwargs": {
                "width": 128,
                "height": 160,
                "refresh": LCD_REFRESH,
                "bgr": LCD_BGR,
                "byte_swap": LCD_BYTE_SWAP,
                "hmirror": LCD_HMIRROR,
                "vflip": LCD_VFLIP,
            },
        }

    if kind == "touch_lcd_2p3":
        return {
            "sensor_framesize": csi.VGA,
            "telemetry_size": (320, 240),
            "display_hint": image.CENTER | image.SCALE_ASPECT_KEEP | image.BILINEAR,
            "display_kwargs": {
                "width": 320,
                "height": 240,
                "refresh": LCD_REFRESH,
                "bgr": LCD_BGR,
                "byte_swap": LCD_BYTE_SWAP,
                "vflip": LCD_VFLIP,
                "hmirror": LCD_HMIRROR,
            },
        }

    raise ValueError("Unsupported DISPLAY_KIND: %s" % kind)


def apply_backlight(lcd, percent):
    # Without an explicit PWM/DAC backlight controller, SPIDisplay falls back
    # to a GPIO backlight where any non-zero value simply means "on".
    try:
        lcd.backlight(1 if percent > 0 else 0)
    except Exception:
        pass


def get_member_value(obj, name):
    value = getattr(obj, name)
    return value() if callable(value) else value


def crc8(data):
    crc = 0

    for value in data:
        crc ^= value
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ 0x07) & 0xFF
            else:
                crc = (crc << 1) & 0xFF

    return crc


def build_vision_frame(x_offset, radius, flags, sequence):
    frame = bytearray(9)
    x_word = int(x_offset) & 0xFFFF
    r_word = int(max(0, radius)) & 0xFFFF

    frame[0] = VISION_SOF1
    frame[1] = VISION_SOF2
    frame[2] = x_word & 0xFF
    frame[3] = (x_word >> 8) & 0xFF
    frame[4] = r_word & 0xFF
    frame[5] = (r_word >> 8) & 0xFF
    frame[6] = flags & 0xFF
    frame[7] = sequence & 0xFF
    frame[8] = crc8(frame[2:8])

    return frame


def scale_to_telemetry(value, src_size, dst_size):
    return int((value * dst_size) / src_size)


def clamp(value, lower, upper):
    return max(lower, min(upper, value))


def classify_red_circle(frame, telemetry_size):
    best_blob = None
    best_score = -1.0
    telemetry_width, _ = telemetry_size

    for blob in frame.find_blobs(
        RED_THRESHOLDS,
        pixels_threshold=PIXELS_THRESHOLD,
        area_threshold=AREA_THRESHOLD,
        merge=True,
        margin=MERGE_MARGIN,
    ):
        width = max(1, get_member_value(blob, "w"))
        height = max(1, get_member_value(blob, "h"))
        pixels = get_member_value(blob, "pixels")
        aspect_ratio = min(width, height) / max(width, height)
        roundness = get_member_value(blob, "roundness")
        density = get_member_value(blob, "density")
        compactness = get_member_value(blob, "compactness")
        radius_est = (width + height) / 4.0
        ideal_disc_area = max(1.0, 3.14159 * radius_est * radius_est)
        disc_fill = pixels / ideal_disc_area

        # The real target is a red center disc with a white printed logo and a
        # transparent outer ring. So we want a large, roughly round red core and
        # should tolerate limited holes inside the red region.
        if pixels < MIN_RED_PIXELS:
            continue

        if aspect_ratio < MIN_ASPECT_RATIO:
            continue

        if roundness < MIN_ROUNDNESS:
            continue

        if density < MIN_DENSITY:
            continue

        if compactness < MIN_COMPACTNESS:
            continue

        if disc_fill < MIN_DISC_FILL:
            continue

        score = (
            (roundness * 0.30)
            + (aspect_ratio * 0.20)
            + (clamp(disc_fill, 0.0, 1.20) * 0.20)
            + (clamp(density / 0.80, 0.0, 1.0) * 0.10)
            + (clamp(compactness / 0.75, 0.0, 1.0) * 0.10)
            + (clamp(pixels / 4000.0, 0.0, 1.0) * 0.10)
        )
        if score > best_score:
            best_score = score
            best_blob = blob

    if best_blob is None:
        return None

    blob_w = get_member_value(best_blob, "w")
    blob_h = get_member_value(best_blob, "h")
    blob_cx = get_member_value(best_blob, "cx")
    blob_cy = get_member_value(best_blob, "cy")
    blob_rect = get_member_value(best_blob, "rect")
    roundness = get_member_value(best_blob, "roundness")
    density = get_member_value(best_blob, "density")
    pixels = get_member_value(best_blob, "pixels")
    raw_radius = int((blob_w + blob_h) / 4)
    disc_fill = pixels / max(1.0, 3.14159 * raw_radius * raw_radius)

    radius = scale_to_telemetry(raw_radius, frame.width(), telemetry_width)
    x_offset = scale_to_telemetry(
        blob_cx - (frame.width() / 2),
        frame.width(),
        telemetry_width,
    )
    flags = VISION_FLAG_VALID | VISION_FLAG_RED_FOUND

    if (roundness >= 0.50) and (density >= 0.35) and (disc_fill >= 0.65):
        flags |= VISION_FLAG_CIRCLE_LIKE

    return {
        "cx": blob_cx,
        "cy": blob_cy,
        "rect": blob_rect,
        "raw_radius": raw_radius,
        "x_offset": x_offset,
        "radius": radius,
        "flags": flags,
        "score": best_score,
    }


def main():
    cfg = build_display_config(DISPLAY_KIND)

    cam = csi.CSI()
    cam.reset()
    cam.pixformat(csi.RGB565)
    cam.framesize(PREVIEW_SENSOR_FRAMESIZE)
    cam.snapshot(time=1000)
    cam.auto_gain(False)
    cam.auto_whitebal(False)
    cam.snapshot(time=200)

    lcd = None
    if ENABLE_LCD:
        lcd = display.SPIDisplay(**cfg["display_kwargs"])
        apply_backlight(lcd, BACKLIGHT_PERCENT)
    uart = UART(UART_PORT, baudrate=UART_BAUD)

    clock = time.clock()
    sequence = 0

    while True:
        clock.tick()
        frame = cam.snapshot()
        result = classify_red_circle(frame, cfg["telemetry_size"])
        flags = 0
        x_offset = 0
        radius = 0

        if result is not None:
            blob_cx = result["cx"]
            blob_cy = result["cy"]
            raw_radius = result["raw_radius"]
            flags = result["flags"]
            x_offset = result["x_offset"]
            radius = result["radius"]

            frame.draw_rectangle(result["rect"], color=(255, 0, 0), thickness=2)
            frame.draw_circle((blob_cx, blob_cy, raw_radius), color=(0, 255, 0), thickness=2)
            frame.draw_cross(blob_cx, blob_cy, color=(255, 255, 0), size=10, thickness=2)
            frame.draw_string(
                2,
                22,
                "DX:%+d R:%d" % (x_offset, radius),
                color=(255, 255, 255),
                scale=2,
            )
        else:
            frame.draw_string(2, 22, "DX:--- R:---", color=(255, 255, 255), scale=2)

        uart.write(build_vision_frame(x_offset, radius, flags, sequence))
        sequence = (sequence + 1) & 0xFF

        frame.draw_string(2, 2, "FPS: %.1f" % clock.fps(), color=(255, 255, 255), scale=2)
        if lcd is not None:
            lcd.write(frame, hint=cfg["display_hint"])

        if PRINT_FPS_TO_REPL:
            print("fps:", clock.fps())

        if PRINT_VISION_TO_REPL:
            print("vision:", x_offset, radius, flags, sequence)


main()
