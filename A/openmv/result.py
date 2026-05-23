RESULT_KEYS = (
    "ok",
    "mode",
    "id",
    "x",
    "y",
    "w",
    "h",
    "score",
    "area",
    "angle",
    "payload",
    "fps",
    "miss",
    "error",
)

RESULT_DEFAULTS = {
    "ok": False,
    "mode": None,
    "id": 0,
    "x": 0,
    "y": 0,
    "w": 0,
    "h": 0,
    "score": 0,
    "area": 0,
    "angle": 0,
    "payload": None,
    "fps": 0,
    "miss": 0,
    "error": None,
}


def make_result(ok=False, mode=None, id=0, x=0, y=0, w=0, h=0, score=0,
                area=0, angle=0, payload=None, fps=0, miss=0, error=None):
    return {
        "ok": bool(ok),
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


def no_target(mode=None, miss=1, fps=0):
    return make_result(ok=False, mode=mode, fps=fps, miss=miss, error=None)


def error_result(message, mode=None, fps=0, miss=1):
    return make_result(ok=False, mode=mode, fps=fps, miss=miss, error=str(message))


def copy_result(result):
    copied = {}
    source = result or {}
    for key in RESULT_KEYS:
        copied[key] = source.get(key, RESULT_DEFAULTS[key])
    copied["ok"] = bool(copied["ok"])
    return copied
