import csi
import display
import gc
import image
import time
from machine import UART

try:
    import ml
    from ml.postprocessing.mediapipe import BlazeFace
except Exception:
    ml = None
    BlazeFace = None


# Set this to match the official screen you actually attached.
# Supported values:
# - "lcd_1p8"        -> OpenMV LCD Shield, 128x160, backlight on P5
# - "touch_lcd_2p3"  -> OpenMV Touch LCD Shield, 320x240, backlight on P6
DISPLAY_KIND = "lcd_1p8"
ENABLE_LCD = True
PREVIEW_SENSOR_FRAMESIZE = csi.QVGA
LCD_REFRESH = 20
LCD_BGR = False
LCD_BYTE_SWAP = False
LCD_HMIRROR = True
LCD_VFLIP = True

BACKLIGHT_PERCENT = 80
PRINT_FPS_TO_REPL = False
PRINT_VISION_TO_REPL = False
REPL_LOG_INTERVAL_MS = 1000

DETECT_EVERY_N_FRAMES = 3
DISPLAY_EVERY_N_FRAMES = 2
GC_EVERY_N_FRAMES = 10
RESULT_STALE_MS = 250
DRAW_LANDMARKS = False

# Touch LCD Shield uses P2/P3/P4/P5/P6, so UART7 (P14/P13) avoids conflicts.
UART_PORT = 7
UART_BAUD = 115200

# 11-byte vision frame:
#   [0]  0xA5
#   [1]  0x5A
#   [2-3] x_offset (int16 LE)
#   [4-5] y_offset (int16 LE)
#   [6-7] width    (uint16 LE)
#   [8]  flags
#   [9]  sequence
#   [10] crc8 over bytes [2..9] (poly 0x07, init 0x00)
VISION_SOF1 = 0xA5
VISION_SOF2 = 0x5A
VISION_FLAG_VALID = 0x01
VISION_FLAG_FACE_FOUND = 0x02
VISION_FLAG_FACE_GOOD = 0x04
VISION_FRAME_LEN = 11

# Prefer NPU face detection on the N6. If model loading or inference fails,
# fall back to the built-in frontal-face Haar cascade.
USE_BLAZEFACE = True
BLAZEFACE_MODEL_PATH = "/rom/blazeface_front_128.tflite"
BLAZEFACE_THRESHOLD = 0.45
BLAZEFACE_GOOD_SCORE = 0.70

USE_HAAR_FALLBACK = True
HAAR_CASCADE_CANDIDATES = (
    "/rom/haarcascade_frontalface.cascade",
    "frontalface",
)
HAAR_STAGES = 25
HAAR_THRESHOLD = 0.75
HAAR_SCALE = 1.35
HAAR_GOOD_MIN_WIDTH = 40

FACE_MIN_WIDTH = 24
FACE_MIN_HEIGHT = 24

# Camera tuning.
CAMERA_CONTRAST = 2
CAMERA_BRIGHTNESS = 0
CAMERA_SATURATION = 0
CAMERA_AUTO_GAIN = True
CAMERA_AUTO_WHITEBAL = True
CAMERA_AUTO_EXPOSURE = True
CAMERA_EXPOSURE_US = 10000


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
            "display_hint": image.CENTER | image.SCALE_ASPECT_KEEP
            | image.BILINEAR,
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


def crc8(data, start=0, end=None):
    crc = 0
    if end is None:
        end = len(data)

    for index in range(start, end):
        value = data[index]
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
    return fill_vision_frame(frame, x_offset, y_offset, width, flags, sequence)


def fill_vision_frame(frame, x_offset, y_offset, width, flags, sequence):
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
    frame[10] = crc8(frame, 2, 10)

    return frame


def ticks_ms():
    try:
        return time.ticks_ms()
    except AttributeError:
        return int(time.time() * 1000)


def ticks_diff(now_ms, start_ms):
    try:
        return time.ticks_diff(now_ms, start_ms)
    except AttributeError:
        return now_ms - start_ms


def scale_to_telemetry(value, src_size, dst_size):
    return int((value * dst_size) / src_size)


def face_rect_score(rect, score=0.0):
    x, y, w, h = rect
    if w < FACE_MIN_WIDTH or h < FACE_MIN_HEIGHT:
        return -1.0
    aspect = min(w, h) / max(w, h)
    if aspect < 0.55:
        return -1.0
    return float(score) * 1000.0 + (w * h)


def load_face_detectors():
    detector = {
        "model": None,
        "haar": None,
    }

    if USE_BLAZEFACE and ml is not None and BlazeFace is not None:
        try:
            detector["model"] = ml.Model(
                BLAZEFACE_MODEL_PATH,
                postprocess=BlazeFace(threshold=BLAZEFACE_THRESHOLD),
            )
        except Exception as exc:
            print("blazeface load failed:", exc)

    if USE_HAAR_FALLBACK:
        last_exc = None
        for cascade_path in HAAR_CASCADE_CANDIDATES:
            try:
                detector["haar"] = image.HaarCascade(
                    cascade_path, stages=HAAR_STAGES
                )
                break
            except Exception as exc:
                last_exc = exc

        if detector["haar"] is None and last_exc is not None:
            print("haar load failed:", last_exc)

    gc.collect()
    return detector


def detect_with_blazeface(frame, detector):
    model = detector.get("model")
    if model is None:
        return None

    best = None
    best_score = -1.0

    try:
        predictions = model.predict([frame])
    except Exception as exc:
        print("blazeface predict failed:", exc)
        detector["model"] = None
        return None

    for rect, score, keypoints in predictions:
        score_value = float(score)
        score_key = face_rect_score(rect, score_value)
        if score_key < 0:
            continue
        if score_key > best_score:
            best_score = score_key
            x, y, w, h = rect
            best = {
                "kind": "BLAZE",
                "rect": (int(x), int(y), int(w), int(h)),
                "cx": x + (w / 2.0),
                "cy": y + (h / 2.0),
                "raw_width": int(w),
                "raw_height": int(h),
                "score": score_value,
                "keypoints": keypoints if DRAW_LANDMARKS else None,
                "is_good": score_value >= BLAZEFACE_GOOD_SCORE,
            }

    return best


def detect_with_haar(frame, detector):
    cascade = detector.get("haar")
    if cascade is None:
        return None

    try:
        faces = frame.find_features(
            cascade,
            threshold=HAAR_THRESHOLD,
            scale=HAAR_SCALE,
        )
    except Exception as exc:
        print("haar predict failed:", exc)
        detector["haar"] = None
        return None

    best = None
    best_area = -1

    for rect in faces:
        x, y, w, h = rect
        if w < FACE_MIN_WIDTH or h < FACE_MIN_HEIGHT:
            continue
        aspect = min(w, h) / max(w, h)
        if aspect < 0.50:
            continue
        area = w * h
        if area > best_area:
            best_area = area
            best = {
                "kind": "HAAR",
                "rect": (int(x), int(y), int(w), int(h)),
                "cx": x + (w / 2.0),
                "cy": y + (h / 2.0),
                "raw_width": int(w),
                "raw_height": int(h),
                "score": 0.0,
                "keypoints": None,
                "is_good": w >= HAAR_GOOD_MIN_WIDTH,
            }

    return best


def classify_face(frame, telemetry_size, detector):
    detection = detect_with_blazeface(frame, detector)
    if detection is None:
        detection = detect_with_haar(frame, detector)
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

    flags = VISION_FLAG_VALID | VISION_FLAG_FACE_FOUND
    if detection["is_good"]:
        flags |= VISION_FLAG_FACE_GOOD
    detection["flags"] = flags

    return detection


def draw_face_overlay(frame, result):
    if result is None:
        frame.draw_string(2, 22, "no face",
                          color=(255, 255, 255), scale=2)
        return

    frame.draw_rectangle(result["rect"], color=(255, 0, 0), thickness=2)
    frame.draw_cross(int(result["cx"]), int(result["cy"]),
                     color=(255, 255, 0), size=10, thickness=2)

    keypoints = result.get("keypoints")
    if keypoints is not None:
        try:
            for point in keypoints:
                frame.draw_cross(int(point[0]), int(point[1]),
                                 color=(0, 255, 255), size=5, thickness=1)
        except Exception:
            pass

    frame.draw_string(
        2,
        22,
        "%s DX:%+d DY:%+d W:%d" % (
            result["kind"],
            result["x_offset"],
            result["y_offset"],
            result["width_out"],
        ),
        color=(255, 255, 255),
        scale=2,
    )


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

    if not CAMERA_AUTO_EXPOSURE:
        safe_call(cam, "auto_exposure", False)
        safe_call(cam, "exposure_us", CAMERA_EXPOSURE_US)

    cam.snapshot(time=200)


def main():
    cfg = build_display_config(DISPLAY_KIND)
    detector = load_face_detectors()

    cam = csi.CSI()
    configure_camera(cam)

    lcd = None
    if ENABLE_LCD:
        lcd = display.SPIDisplay(**cfg["display_kwargs"])
        apply_backlight(lcd, BACKLIGHT_PERCENT)
    uart = UART(UART_PORT, baudrate=UART_BAUD)

    clock = time.clock()
    sequence = 0
    frame_count = 0
    last_result = None
    last_result_ms = ticks_ms() - RESULT_STALE_MS
    last_log_ms = ticks_ms() - REPL_LOG_INTERVAL_MS
    vision_frame = bytearray(VISION_FRAME_LEN)

    gc.collect()

    while True:
        clock.tick()
        frame_count += 1
        frame = cam.snapshot()
        now_ms = ticks_ms()
        result = None

        if (frame_count == 1) or ((frame_count % DETECT_EVERY_N_FRAMES) == 0):
            result = classify_face(frame, cfg["telemetry_size"], detector)
            if result is not None:
                last_result = result
                last_result_ms = now_ms

        effective_result = last_result
        if (effective_result is not None) and (
            ticks_diff(now_ms, last_result_ms) > RESULT_STALE_MS
        ):
            last_result = None
            effective_result = None

        flags = 0
        x_offset = 0
        y_offset = 0
        width_out = 0
        kind_label = "---"

        if effective_result is not None:
            flags = effective_result["flags"]
            x_offset = effective_result["x_offset"]
            y_offset = effective_result["y_offset"]
            width_out = effective_result["width_out"]
            kind_label = effective_result["kind"]

        current_sequence = sequence
        uart.write(fill_vision_frame(vision_frame, x_offset, y_offset, width_out,
                                     flags, current_sequence))
        sequence = (sequence + 1) & 0xFF

        should_draw = (lcd is not None) and (
            (frame_count == 1) or ((frame_count % DISPLAY_EVERY_N_FRAMES) == 0)
        )
        if should_draw:
            draw_face_overlay(frame, effective_result)
            frame.draw_string(2, 2, "FPS: %.1f" % clock.fps(),
                              color=(255, 255, 255), scale=2)
            lcd.write(frame, hint=cfg["display_hint"])

        if (PRINT_FPS_TO_REPL or PRINT_VISION_TO_REPL) and (
            ticks_diff(now_ms, last_log_ms) >= REPL_LOG_INTERVAL_MS
        ):
            last_log_ms = now_ms

            if PRINT_FPS_TO_REPL:
                print("fps:", clock.fps())

            if PRINT_VISION_TO_REPL:
                print("vision: kind=%s x=%+d y=%+d w=%d flags=0x%02X seq=%d"
                      % (kind_label, x_offset, y_offset, width_out,
                         flags, current_sequence))

        if (frame_count % GC_EVERY_N_FRAMES) == 0:
            gc.collect()


main()
