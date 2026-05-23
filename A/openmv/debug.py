try:
    import time
except ImportError:
    time = None

try:
    from config import (
        DEBUG,
        DEBUG_DRAW,
        DEBUG_FPS,
        DEBUG_SAVE_IMAGE,
        DEBUG_SAVE_INTERVAL_MS,
        DEBUG_IMAGE_PREFIX,
    )
except ImportError:
    DEBUG = False
    DEBUG_DRAW = False
    DEBUG_FPS = False
    DEBUG_SAVE_IMAGE = False
    DEBUG_SAVE_INTERVAL_MS = 1000
    DEBUG_IMAGE_PREFIX = "debug"


_clock = time.clock() if DEBUG_FPS and time and hasattr(time, "clock") else None
_last_save_ms = 0
_save_index = 0


def _millis():
    if time is None:
        return 0
    ticks_ms = getattr(time, "ticks_ms", None)
    if ticks_ms:
        return ticks_ms()
    return int(time.time() * 1000)


def _ticks_diff(now, old):
    if time is None:
        return now - old
    ticks_diff = getattr(time, "ticks_diff", None)
    if ticks_diff:
        return ticks_diff(now, old)
    return now - old


def log(*args):
    if DEBUG:
        print(*args)


def fps_tick():
    if _clock is None:
        return 0
    _clock.tick()
    return _clock.fps()


def draw_rect(img, rect, color=(255, 0, 0), thickness=2):
    if DEBUG_DRAW and img and rect:
        img.draw_rectangle(rect, color=color, thickness=thickness)
    return img


def draw_circle(img, x, y=None, r=None, color=(0, 255, 0), thickness=2):
    if not DEBUG_DRAW or not img:
        return img
    if y is None and r is None:
        img.draw_circle(x, color=color, thickness=thickness)
    else:
        img.draw_circle(int(x), int(y), int(r), color=color, thickness=thickness)
    return img


def draw_blob(img, blob, color=(255, 0, 0), thickness=2, cross=True):
    if not DEBUG_DRAW or not img or not blob:
        return img
    try:
        rect = blob.rect()
        cx = blob.cx()
        cy = blob.cy()
    except AttributeError:
        rect = blob[0:4]
        cx = rect[0] + rect[2] // 2
        cy = rect[1] + rect[3] // 2
    img.draw_rectangle(rect, color=color, thickness=thickness)
    if cross:
        img.draw_cross(cx, cy, color=color)
    return img


def draw_tag(img, tag, color=(0, 255, 0), thickness=2):
    if not DEBUG_DRAW or not img or not tag:
        return img
    try:
        corners = tag.corners()
        cx = tag.cx()
        cy = tag.cy()
        tag_id = tag.id()
    except AttributeError:
        return img
    for index in range(4):
        x1, y1 = corners[index]
        x2, y2 = corners[(index + 1) % 4]
        img.draw_line((x1, y1, x2, y2), color=color, thickness=thickness)
    img.draw_cross(cx, cy, color=color)
    img.draw_string(cx + 4, cy + 4, str(tag_id), color=color)
    return img


def draw_qrcode(img, code, color=(255, 255, 0), thickness=2):
    if not DEBUG_DRAW or not img or not code:
        return img
    try:
        rect = code.rect()
        payload = code.payload()
    except AttributeError:
        return img
    img.draw_rectangle(rect, color=color, thickness=thickness)
    img.draw_string(rect[0], rect[1] - 10, payload, color=color)
    return img


def draw_result(img, result, color=(255, 0, 0)):
    if not DEBUG_DRAW or not img or result is None:
        return img
    if isinstance(result, dict):
        if not result.get("ok"):
            return img
        w = int(result.get("w", 0) or 0)
        h = int(result.get("h", 0) or 0)
        x = int(result.get("left", result.get("x", 0) - w // 2) or 0)
        y = int(result.get("top", result.get("y", 0) - h // 2) or 0)
        if w and h:
            img.draw_rectangle((x, y, w, h), color=color)
        img.draw_cross(int(result.get("x", 0) or 0), int(result.get("y", 0) or 0), color=color)
        return img
    if isinstance(result, tuple) and len(result) == 4:
        return draw_rect(img, result, color=color)
    if isinstance(result, (list, tuple)):
        for item in result:
            draw_result(img, item, color=color)
        return img
    if hasattr(result, "payload"):
        return draw_qrcode(img, result, color=color)
    if hasattr(result, "corners") and hasattr(result, "id"):
        return draw_tag(img, result, color=color)
    if hasattr(result, "rect") and hasattr(result, "cx"):
        return draw_blob(img, result, color=color)
    return img


def draw(img, result=None):
    return draw_result(img, result)


def show(img, result=None):
    return draw_result(img, result)


def render(img, result=None):
    return draw_result(img, result)


def maybe_save_image(img, prefix=None, interval_ms=None):
    global _last_save_ms, _save_index
    if not DEBUG_SAVE_IMAGE or not img:
        return None

    now = _millis()
    wait_ms = DEBUG_SAVE_INTERVAL_MS if interval_ms is None else interval_ms
    if wait_ms and _ticks_diff(now, _last_save_ms) < wait_ms:
        return None

    name = "%s_%04d.jpg" % (prefix or DEBUG_IMAGE_PREFIX, _save_index)
    try:
        img.save(name)
    except Exception as err:
        log("save image failed:", err)
        return None
    _last_save_ms = now
    _save_index += 1
    return name
