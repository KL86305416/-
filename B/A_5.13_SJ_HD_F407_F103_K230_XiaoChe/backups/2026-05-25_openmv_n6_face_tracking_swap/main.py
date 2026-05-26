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
PRINT_FPS_TO_REPL = False
PRINT_VISION_TO_REPL = True

# Touch LCD Shield uses P2/P3/P4/P5/P6, so UART7 (P14/P13) avoids conflicts.
UART_PORT = 7
UART_BAUD = 115200

# 11-byte vision frame (extended from the original 9-byte schema):
#   [0]  0xA5
#   [1]  0x5A
#   [2-3] x_offset (int16 LE)  -- rect center X minus image center X
#   [4-5] y_offset (int16 LE)  -- rect center Y minus image center Y
#   [6-7] width    (uint16 LE) -- rect width mapped into telemetry frame
#   [8]  flags
#   [9]  sequence
#   [10] crc8 over bytes [2..9] (poly 0x07, init 0x00)
VISION_SOF1 = 0xA5
VISION_SOF2 = 0x5A
VISION_FLAG_VALID = 0x01
VISION_FLAG_RECT_FOUND = 0x02
VISION_FLAG_RECT_GOOD = 0x04
VISION_FRAME_LEN = 11

# ---- Primary detector: image.find_rects (Hough-style geometry detector).
# Lower threshold = more sensitive (more candidates, more false positives).
# Bump it up if the LCD overlay flickers across spurious rectangles.
RECT_THRESHOLD = 8000
RECT_MIN_MAGNITUDE = 6000
RECT_GOOD_MAGNITUDE = 18000
RECT_MIN_SIDE = 18
RECT_MIN_ASPECT = 0.30
RECT_MAX_COVERAGE = 0.95
# Bounding-box coverage of the frame above which a candidate is ignored.

# ---- Fallback detector: morphology + find_blobs on a black mask.
# Only consulted if find_rects returns nothing on the current frame. LAB
# thresholds — L_max is the most important knob.  Raise it (toward 70-90) for
# dim scenes, lower it (toward 30-40) when bright backgrounds bleed in.
BLACK_THRESHOLDS = [
    (0, 70, -30, 30, -30, 30),
]
BLOB_PIXELS_THRESHOLD = 250
BLOB_AREA_THRESHOLD = 250
BLOB_MERGE_MARGIN = 6
BLOB_MIN_PIXELS = 350
BLOB_MIN_SIDE = 18
BLOB_MIN_ASPECT = 0.30
BLOB_MAX_DENSITY = 0.80
BLOB_MIN_DENSITY = 0.03

# Camera tuning to help thresholding stay stable.
CAMERA_CONTRAST = 2
CAMERA_BRIGHTNESS = 0
CAMERA_SATURATION = -1
CAMERA_AUTO_GAIN = False
CAMERA_AUTO_WHITEBAL = False
CAMERA_AUTO_EXPOSURE = False
CAMERA_EXPOSURE_US = 8000


def build_display_config(kind):
    if kind == "lcd_1p8":
        return {
            "telemetry_size": (128, 160),
            "display_hint": image.CENTER | image.SCALE_ASPECT_KEEP,
            "display_kwargs": {
                "hmirror": LCD_HMIRROR,
                "vflip": LCD_VFLIP,
            },
        }

    if kind == "touch_lcd_2p3":
        return {
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


def safe_call(obj, name, *args, **kwargs):
    method = getattr(obj, name, None)
    if method is None:
        return None
    try:
        return method(*args, **kwargs)
    except Exception:
        return None


def get_member_value(obj, name):
    try:
        value = getattr(obj, name)
    except AttributeError:
        return None
    if callable(value):
        try:
            return value()
        except Exception:
            return None
    return value


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


def pack_int16(value):
    value = int(value)
    if value < -32768:
        value = -32768
    elif value > 32767:
        value = 32767
    return value & 0xFFFF


def pack_uint16(value):
    value = int(value)
    if value < 0:
        value = 0
    elif value > 65535:
        value = 65535
    return value


def build_vision_frame(x_offset, y_offset, width, flags, sequence):
    frame = bytearray(VISION_FRAME_LEN)
    x_word = pack_int16(x_offset)
    y_word = pack_int16(y_offset)
    w_word = pack_uint16(width)

    frame[0] = VISION_SOF1
    frame[1] = VISION_SOF2
    frame[2] = x_word & 0xFF
    frame[3] = (x_word >> 8) & 0xFF
    frame[4] = y_word & 0xFF
    frame[5] = (y_word >> 8) & 0xFF
    frame[6] = w_word & 0xFF
    frame[7] = (w_word >> 8) & 0xFF
    frame[8] = flags & 0xFF
    frame[9] = sequence & 0xFF
    frame[10] = crc8(frame[2:10])

    return frame


def scale_to_telemetry(value, src_size, dst_size):
    return int((value * dst_size) / src_size)


def clamp(value, lower, upper):
    return max(lower, min(upper, value))


def detect_with_find_rects(frame):
    frame_w = frame.width()
    frame_h = frame.height()
    coverage_limit_w = frame_w * RECT_MAX_COVERAGE
    coverage_limit_h = frame_h * RECT_MAX_COVERAGE

    try:
        rects = frame.find_rects(threshold=RECT_THRESHOLD)
    except Exception:
        return None

    best = None
    best_score = -1.0

    for rect in rects:
        magnitude = get_member_value(rect, "magnitude") or 0
        if magnitude < RECT_MIN_MAGNITUDE:
            continue

        x = get_member_value(rect, "x") or 0
        y = get_member_value(rect, "y") or 0
        w = get_member_value(rect, "w") or 0
        h = get_member_value(rect, "h") or 0
        if w < RECT_MIN_SIDE or h < RECT_MIN_SIDE:
            continue
        if w >= coverage_limit_w and h >= coverage_limit_h:
            continue

        aspect = min(w, h) / max(w, h) if max(w, h) > 0 else 0
        if aspect < RECT_MIN_ASPECT:
            continue

        score = (
            clamp(magnitude / 30000.0, 0.0, 1.0) * 0.65
            + aspect * 0.25
            + clamp((w * h) / (frame_w * frame_h * 0.4), 0.0, 1.0) * 0.10
        )

        if score > best_score:
            best_score = score
            best = {
                "x": x,
                "y": y,
                "w": w,
                "h": h,
                "magnitude": magnitude,
                "aspect": aspect,
                "corners": get_member_value(rect, "corners"),
                "rect": (x, y, w, h),
                "score": score,
            }

    if best is None:
        return None

    return {
        "kind": "rect",
        "cx": best["x"] + best["w"] / 2.0,
        "cy": best["y"] + best["h"] / 2.0,
        "raw_width": best["w"],
        "raw_height": best["h"],
        "rect": best["rect"],
        "corners": best["corners"],
        "magnitude": best["magnitude"],
        "aspect": best["aspect"],
        "extent": None,
        "density": None,
        "pixels": None,
        "is_good": best["magnitude"] >= RECT_GOOD_MAGNITUDE,
        "score": best["score"],
    }


def detect_with_find_blobs(frame):
    frame_w = frame.width()
    frame_h = frame.height()

    best = None
    best_score = -1.0

    for blob in frame.find_blobs(
        BLACK_THRESHOLDS,
        pixels_threshold=BLOB_PIXELS_THRESHOLD,
        area_threshold=BLOB_AREA_THRESHOLD,
        merge=True,
        margin=BLOB_MERGE_MARGIN,
    ):
        width = max(1, get_member_value(blob, "w"))
        height = max(1, get_member_value(blob, "h"))
        pixels = get_member_value(blob, "pixels") or 0
        if pixels < BLOB_MIN_PIXELS:
            continue
        if width < BLOB_MIN_SIDE or height < BLOB_MIN_SIDE:
            continue
        if width >= int(frame_w * RECT_MAX_COVERAGE) and height >= int(frame_h * RECT_MAX_COVERAGE):
            continue

        aspect = min(width, height) / max(width, height)
        if aspect < BLOB_MIN_ASPECT:
            continue

        density = pixels / float(width * height)
        if density > BLOB_MAX_DENSITY or density < BLOB_MIN_DENSITY:
            continue

        extent = get_member_value(blob, "extent")
        if extent is None:
            extent = density

        score = (
            aspect * 0.35
            + clamp(extent, 0.0, 1.0) * 0.30
            + (1.0 - abs(density - 0.18) / 0.4) * 0.20
            + clamp(pixels / 6000.0, 0.0, 1.0) * 0.15
        )

        if score > best_score:
            best_score = score
            best = {
                "cx": get_member_value(blob, "cx"),
                "cy": get_member_value(blob, "cy"),
                "w": width,
                "h": height,
                "pixels": pixels,
                "density": density,
                "aspect": aspect,
                "extent": extent,
                "rect": get_member_value(blob, "rect"),
                "corners": get_member_value(blob, "min_corners"),
                "score": score,
            }

    if best is None:
        return None

    return {
        "kind": "blob",
        "cx": best["cx"],
        "cy": best["cy"],
        "raw_width": best["w"],
        "raw_height": best["h"],
        "rect": best["rect"],
        "corners": best["corners"],
        "magnitude": None,
        "aspect": best["aspect"],
        "extent": best["extent"],
        "density": best["density"],
        "pixels": best["pixels"],
        "is_good": best["aspect"] >= 0.55 and best["extent"] >= 0.78,
        "score": best["score"],
    }


def classify_black_rectangle(frame, telemetry_size):
    primary = detect_with_find_rects(frame)
    detection = primary if primary is not None else detect_with_find_blobs(frame)
    if detection is None:
        return None

    telemetry_w, telemetry_h = telemetry_size
    frame_w = frame.width()
    frame_h = frame.height()

    detection["x_offset"] = scale_to_telemetry(
        detection["cx"] - (frame_w / 2),
        frame_w,
        telemetry_w,
    )
    detection["y_offset"] = scale_to_telemetry(
        detection["cy"] - (frame_h / 2),
        frame_h,
        telemetry_h,
    )
    detection["width_out"] = scale_to_telemetry(
        detection["raw_width"], frame_w, telemetry_w
    )

    flags = VISION_FLAG_VALID | VISION_FLAG_RECT_FOUND
    if detection.get("is_good"):
        flags |= VISION_FLAG_RECT_GOOD
    detection["flags"] = flags

    return detection


def draw_corners(frame, corners, color):
    if corners is None:
        return

    try:
        last = corners[-1]
        for corner in corners:
            frame.draw_line(
                (last[0], last[1], corner[0], corner[1]),
                color=color,
                thickness=2,
            )
            last = corner
    except Exception:
        pass


def configure_camera(cam):
    cam.reset()
    cam.pixformat(csi.RGB565)
    cam.framesize(PREVIEW_SENSOR_FRAMESIZE)
    cam.snapshot(time=600)

    safe_call(cam, "contrast", CAMERA_CONTRAST)
    safe_call(cam, "brightness", CAMERA_BRIGHTNESS)
    safe_call(cam, "saturation", CAMERA_SATURATION)

    cam.auto_gain(CAMERA_AUTO_GAIN)
    cam.auto_whitebal(CAMERA_AUTO_WHITEBAL)
    safe_call(cam, "auto_exposure", CAMERA_AUTO_EXPOSURE,
              exposure_us=CAMERA_EXPOSURE_US)

    # Fallback for firmwares whose auto_exposure() does not take kwargs.
    if not CAMERA_AUTO_EXPOSURE:
        safe_call(cam, "auto_exposure", False)
        safe_call(cam, "exposure_us", CAMERA_EXPOSURE_US)

    cam.snapshot(time=200)


def main():
    cfg = build_display_config(DISPLAY_KIND)

    cam = csi.CSI()
    configure_camera(cam)

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
        result = classify_black_rectangle(frame, cfg["telemetry_size"])
        flags = 0
        x_offset = 0
        y_offset = 0
        width_out = 0
        kind_label = "---"

        if result is not None:
            flags = result["flags"]
            x_offset = result["x_offset"]
            y_offset = result["y_offset"]
            width_out = result["width_out"]
            kind_label = result["kind"]

            corner_color = (0, 255, 0) if result["kind"] == "rect" \
                           else (0, 255, 255)
            if result["rect"] is not None:
                frame.draw_rectangle(result["rect"], color=(255, 0, 0),
                                     thickness=2)
            draw_corners(frame, result["corners"], corner_color)
            frame.draw_cross(int(result["cx"]), int(result["cy"]),
                             color=(255, 255, 0), size=10, thickness=2)
            frame.draw_string(
                2,
                22,
                "%s DX:%+d DY:%+d W:%d" % (kind_label, x_offset,
                                           y_offset, width_out),
                color=(255, 255, 255),
                scale=2,
            )
        else:
            frame.draw_string(2, 22, "no rect",
                              color=(255, 255, 255), scale=2)

        uart.write(build_vision_frame(x_offset, y_offset, width_out,
                                      flags, sequence))
        sequence = (sequence + 1) & 0xFF

        frame.draw_string(2, 2, "FPS: %.1f" % clock.fps(),
                          color=(255, 255, 255), scale=2)
        if lcd is not None:
            lcd.write(frame, hint=cfg["display_hint"])

        if PRINT_FPS_TO_REPL:
            print("fps:", clock.fps())

        if PRINT_VISION_TO_REPL:
            print("vision: kind=%s x=%+d y=%+d w=%d flags=0x%02X seq=%d"
                  % (kind_label, x_offset, y_offset, width_out,
                     flags, sequence))


main()
