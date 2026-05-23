import sensor
import time
from pyb import UART

try:
    import config
except Exception:
    config = None

try:
    import camera as camera_mod
except Exception:
    camera_mod = None

try:
    import comm as comm_mod
except Exception:
    comm_mod = None

try:
    import protocol as protocol_mod
except Exception:
    protocol_mod = None

try:
    import state as state_mod
except Exception:
    state_mod = None

try:
    import filter as filter_mod
except Exception:
    filter_mod = None

try:
    import blob as blob_mod
except Exception:
    blob_mod = None

try:
    import line as line_mod
except Exception:
    line_mod = None

try:
    import shape as shape_mod
except Exception:
    shape_mod = None

try:
    import tag_code as tag_mod
except Exception:
    tag_mod = None

try:
    import debug as debug_mod
except Exception:
    debug_mod = None

try:
    import guard as guard_mod
except Exception:
    guard_mod = None

try:
    import result as result_mod
except Exception:
    result_mod = None


MODES = ("idle", "red", "green", "blue", "line", "shape", "apriltag", "qrcode", "auto")
MODE_ALIASES = {
    "tag": "apriltag",
    "tags": "apriltag",
    "april": "apriltag",
    "apriltags": "apriltag",
    "qr": "qrcode",
    "qrcodes": "qrcode",
    "code": "qrcode",
    "codes": "qrcode",
    "color": "red",
    "colour": "red",
}
DEFAULT_MODE = getattr(config, "DEFAULT_MODE", "auto") if config else "auto"
UART_PORT = getattr(config, "UART_PORT", 3) if config else 3
UART_BAUDRATE = getattr(config, "UART_BAUDRATE", 115200) if config else 115200
SEND_INTERVAL_MS = getattr(config, "SEND_INTERVAL_MS", 50) if config else 50
THRESHOLDS = getattr(config, "THRESHOLDS", {}) if config else {}
ROIS = getattr(config, "ROIS", {}) if config else {}
MODE_CONFIG = getattr(config, "MODES", {}) if config else {}
DEBUG = bool(getattr(config, "DEBUG", False)) if config else False
DEBUG_DRAW = bool(getattr(config, "DEBUG_DRAW", False)) if config else False


def call_first(module, names, *args):
    if not module:
        return None
    for name in names:
        fn = getattr(module, name, None)
        if fn:
            try:
                return fn(*args)
            except Exception:
                return None
    return None


def normalize_mode(value, fallback="auto"):
    if value is None:
        return fallback
    value = str(value).strip().lower()
    if not value:
        return fallback
    value = MODE_ALIASES.get(value, value)
    return value if value in MODES else fallback


def make_result(ok=False, mode=None, id=0, x=0, y=0, w=0, h=0, score=0,
                area=0, angle=0, payload=None, fps=0, miss=0, error=None):
    if result_mod and hasattr(result_mod, "make_result"):
        try:
            data = result_mod.make_result(ok, mode, id, x, y, w, h, score, area, angle, payload, fps, miss, error)
            if data:
                return data
        except Exception:
            pass
    return {
        "ok": ok,
        "mode": mode,
        "id": id,
        "x": x,
        "y": y,
        "w": w,
        "h": h,
        "score": score,
        "area": area,
        "angle": angle,
        "payload": payload,
        "fps": fps,
        "miss": miss,
        "error": error,
    }


def no_target(mode, miss=1, fps=0):
    if result_mod and hasattr(result_mod, "no_target"):
        try:
            data = result_mod.no_target(mode, miss, fps)
            if data:
                return data
        except Exception:
            pass
    return make_result(False, mode=mode, fps=fps, miss=miss)


def init_camera():
    if call_first(camera_mod, ("init", "setup", "init_camera")):
        return
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames(time=1200)
    sensor.set_auto_gain(False)
    sensor.set_auto_whitebal(False)


def init_uart():
    uart = call_first(comm_mod, ("init", "setup", "init_uart"))
    if uart:
        return uart
    return UART(UART_PORT, UART_BAUDRATE, timeout_char=20)


def mode_item(mode, key, default=None):
    item = MODE_CONFIG.get(mode, {}) if MODE_CONFIG else {}
    return item.get(key, default)


def read_command(uart, current_mode):
    command = call_first(comm_mod, ("read_command", "read_cmd", "recv"), uart)
    if isinstance(command, dict):
        fields = command.get("fields", [])
        cmd = str(command.get("cmd", fields[0] if fields else "")).strip().upper()
        args = command.get("args", fields[1:] if len(fields) > 1 else [])
        if cmd == "MODE" and args:
            return normalize_mode(args[0], current_mode)
        if cmd == "STOP":
            return "idle"
        if cmd in ("START", "PING", "DEBUG", "SAVE", "RESET"):
            return current_mode
        return current_mode
    if command is None and uart and uart.any():
        try:
            command = uart.readline()
        except Exception:
            command = None
    if command is None:
        return current_mode
    if protocol_mod and hasattr(protocol_mod, "parse_frame"):
        try:
            frame = protocol_mod.parse_frame(command)
        except Exception:
            frame = None
        if frame and frame.get("prefix") == getattr(protocol_mod, "PREFIX_COMMAND", "MC"):
            fields = frame.get("fields", [])
            cmd = str(frame.get("cmd", fields[0] if fields else "")).strip().upper()
            args = frame.get("args", fields[1:] if len(fields) > 1 else [])
            if cmd == "MODE" and args:
                return normalize_mode(args[0], current_mode)
            if cmd == "STOP":
                return "idle"
            return current_mode
    if not isinstance(command, str):
        try:
            command = command.decode()
        except Exception:
            command = str(command)
    command = command.strip().lower()
    if command.startswith("mode:"):
        command = command[5:].strip()
    elif command.startswith("mode="):
        command = command[5:].strip()
    elif command.startswith("m:"):
        command = command[2:].strip()
    elif command.startswith("m="):
        command = command[2:].strip()
    return normalize_mode(command, current_mode)


def pick_blob(blobs):
    best = None
    best_area = 0
    for item in blobs:
        area = item.pixels() if hasattr(item, "pixels") else item.w() * item.h()
        if area > best_area:
            best = item
            best_area = area
    return best, best_area


def fallback_blob(img, mode, fps):
    threshold_name = mode_item(mode, "threshold", mode)
    threshold = THRESHOLDS.get(threshold_name)
    if not threshold:
        return no_target(mode, fps=fps)
    roi_name = mode_item(mode, "roi", "full")
    roi = ROIS.get(roi_name, ROIS.get("full", (0, 0, img.width(), img.height())))
    min_area = mode_item(mode, "min_area", 80)
    blobs = img.find_blobs([threshold], roi=roi, pixels_threshold=min_area,
                           area_threshold=min_area, merge=True)
    target, area = pick_blob(blobs)
    if not target:
        return no_target(mode, fps=fps)
    if DEBUG_DRAW:
        img.draw_rectangle(target.rect())
        img.draw_cross(target.cx(), target.cy())
    return make_result(True, mode=mode, id=mode_item(mode, "id", 0),
                       x=target.cx(), y=target.cy(), w=target.w(), h=target.h(),
                       score=area, area=area, fps=fps)


def fallback_line(img, mode, fps):
    threshold = THRESHOLDS.get("black", (0, 35, -30, 30, -30, 30))
    roi = ROIS.get("line", (0, 80, img.width(), img.height() // 2))
    blobs = img.find_blobs([threshold], roi=roi, pixels_threshold=120,
                           area_threshold=120, merge=True)
    target, area = pick_blob(blobs)
    if not target:
        return no_target(mode, fps=fps)
    cx = target.cx()
    cy = target.cy()
    angle = int((cx - img.width() // 2) * 100 / max(1, img.width() // 2))
    if DEBUG_DRAW:
        img.draw_rectangle(target.rect())
        img.draw_line(img.width() // 2, img.height(), cx, cy)
    return make_result(True, mode=mode, id=mode_item(mode, "id", 4),
                       x=cx, y=cy, w=target.w(), h=target.h(), score=area,
                       area=area, angle=angle, fps=fps)


def get_roi(img, mode, default_name="full"):
    roi_name = mode_item(mode, "roi", default_name)
    roi = ROIS.get(roi_name) if ROIS else None
    if roi:
        return roi
    return (0, 0, img.width(), img.height())


def finish_result(data, mode, fps):
    if not data or not isinstance(data, dict):
        return no_target(mode, fps=fps)
    data["mode"] = mode
    data["ok"] = bool(data.get("ok"))
    data["fps"] = data.get("fps", fps)
    data["id"] = data.get("id", mode_item(mode, "id", 0))
    if "cx" in data:
        data["x"] = data.get("cx", data.get("x", 0))
    if "cy" in data:
        data["y"] = data.get("cy", data.get("y", 0))
    if mode == "line" and isinstance(data.get("error"), (int, float)):
        data["angle"] = int(data.get("error"))
        data["error"] = None
    return data


def detect_color(img, mode, fps):
    threshold_name = mode_item(mode, "threshold", mode)
    threshold = THRESHOLDS.get(threshold_name)
    if not threshold:
        return no_target(mode, fps=fps)
    roi = get_roi(img, mode, "center")
    min_area = mode_item(mode, "min_area", 80)
    if blob_mod and hasattr(blob_mod, "detect_color_target"):
        data = blob_mod.detect_color_target(
            img, [threshold], roi=roi,
            pixels_threshold=min_area,
            area_threshold=min_area,
            merge=True,
        )
        return finish_result(data, mode, fps)
    return fallback_blob(img, mode, fps)


def detect_line(img, mode, fps):
    threshold = THRESHOLDS.get("black", (0, 35, -30, 30, -30, 30))
    if line_mod and hasattr(line_mod, "line_follow_weighted"):
        data = line_mod.line_follow_weighted(
            img, [threshold],
            pixels_threshold=mode_item(mode, "min_area", 120),
            area_threshold=mode_item(mode, "min_area", 120),
        )
        return finish_result(data, mode, fps)
    return fallback_line(img, mode, fps)


def detect_shape(img, mode, fps):
    roi = get_roi(img, mode, "center")
    if shape_mod and hasattr(shape_mod, "detect_circles"):
        circles = shape_mod.detect_circles(img, roi=roi)
        if circles:
            circle = circles[0]
            data = shape_mod.circle_to_result(circle) if hasattr(shape_mod, "circle_to_result") else None
            return finish_result(data, mode, fps)
    if shape_mod and hasattr(shape_mod, "detect_rects"):
        rects = shape_mod.detect_rects(img, roi=roi)
        if rects:
            rect = rects[0]
            data = shape_mod.rect_to_result(rect) if hasattr(shape_mod, "rect_to_result") else None
            return finish_result(data, mode, fps)
    return no_target(mode, fps=fps)


def detect_apriltag(img, mode, fps):
    roi = get_roi(img, mode, "full")
    if tag_mod and hasattr(tag_mod, "detect_apriltags"):
        tags = tag_mod.detect_apriltags(img, roi=roi)
        if tags:
            data = tag_mod.tag_to_result(tags[0]) if hasattr(tag_mod, "tag_to_result") else None
            return finish_result(data, mode, fps)
    return no_target(mode, fps=fps)


def detect_qrcode(img, mode, fps):
    roi = get_roi(img, mode, "full")
    if tag_mod and hasattr(tag_mod, "detect_qrcodes"):
        codes = tag_mod.detect_qrcodes(img, roi=roi)
        if codes:
            data = tag_mod.qrcode_to_result(codes[0]) if hasattr(tag_mod, "qrcode_to_result") else None
            return finish_result(data, mode, fps)
    return no_target(mode, fps=fps)


def detect(img, mode, fps):
    mode = normalize_mode(mode)
    guarded = call_first(guard_mod, ("before_detect", "check", "allow"), img, mode)
    if guarded is False:
        return no_target(mode, fps=fps)
    if mode == "idle":
        return no_target(mode, miss=0, fps=fps)
    if mode in ("red", "green", "blue"):
        return detect_color(img, mode, fps)
    if mode == "line":
        return detect_line(img, mode, fps)
    if mode == "shape":
        return detect_shape(img, mode, fps)
    if mode == "apriltag":
        return detect_apriltag(img, mode, fps)
    if mode == "qrcode":
        return detect_qrcode(img, mode, fps)
    if mode == "auto":
        for item in ("qrcode", "apriltag", "shape", "line", "red", "green", "blue"):
            data = detect(img, item, fps)
            if data and data.get("ok"):
                data["mode"] = "auto"
                return data
        return no_target(mode, fps=fps)
    return no_target(mode, fps=fps)


def format_packet(data):
    if comm_mod:
        packet = call_first(comm_mod, ("format_result", "pack", "encode"), data)
        if packet is not None:
            return packet
    if protocol_mod and hasattr(protocol_mod, "build_frame"):
        try:
            return protocol_mod.build_frame(
                getattr(protocol_mod, "PREFIX_VISION", "MV"),
                "RESULT",
                data.get("mode", ""),
                1 if data.get("ok") else 0,
                data.get("id", 0) or 0,
                data.get("x", 0) or 0,
                data.get("y", 0) or 0,
                data.get("w", 0) or 0,
                data.get("h", 0) or 0,
                data.get("score", 0) or 0,
                data.get("area", 0) or 0,
                data.get("angle", 0) or 0,
                data.get("payload", "") or "",
                data.get("fps", 0) or 0,
                data.get("miss", 0) or 0,
                data.get("error", "") or "",
            )
        except Exception:
            pass
    return "%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%s,%d,%d,%s\n" % (
        1 if data.get("ok") else 0,
        data.get("mode", ""),
        data.get("id", 0) or 0,
        data.get("x", 0) or 0,
        data.get("y", 0) or 0,
        data.get("w", 0) or 0,
        data.get("h", 0) or 0,
        data.get("score", 0) or 0,
        data.get("area", 0) or 0,
        data.get("angle", 0) or 0,
        data.get("payload", "") or "",
        data.get("fps", 0) or 0,
        data.get("miss", 0) or 0,
        data.get("error", "") or "",
    )


def send_result(uart, data):
    if comm_mod and call_first(comm_mod, ("send_result", "send"), uart, data):
        return
    uart.write(format_packet(data))


def draw_debug(img, data):
    if debug_mod:
        call_first(debug_mod, ("draw_result", "draw", "show", "render"), img, data)
    elif DEBUG:
        print(data)


def main():
    init_camera()
    uart = init_uart()
    clock = time.clock()
    mode = normalize_mode(DEFAULT_MODE, "auto")
    last_send = time.ticks_ms()
    result_filter = filter_mod.ResultFilter() if filter_mod and hasattr(filter_mod, "ResultFilter") else None
    call_first(state_mod, ("init", "reset"))
    while True:
        clock.tick()
        mode = read_command(uart, mode)
        img = camera_mod.snapshot() if camera_mod and hasattr(camera_mod, "snapshot") else sensor.snapshot()
        fps = int(clock.fps())
        try:
            data = detect(img, mode, fps)
        except Exception as exc:
            data = make_result(False, mode=mode, fps=fps, error=str(exc), miss=1)
        if result_filter:
            data = result_filter.update(data)
        call_first(state_mod, ("update", "set_result"), data)
        call_first(guard_mod, ("after_detect", "watchdog", "feed"), data)
        draw_debug(img, data)
        now = time.ticks_ms()
        if time.ticks_diff(now, last_send) >= SEND_INTERVAL_MS:
            send_result(uart, data)
            last_send = now


main()
