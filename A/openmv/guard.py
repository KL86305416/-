ERR_NONE = 0
ERR_EXCEPTION = 1
ERR_CAMERA = 2
ERR_UART = 3
ERR_PROTOCOL = 4
ERR_STATE = 5
ERR_NO_TARGET = 6

_last_error = None
_miss_count = 0


def safe_call(func, *args, **kwargs):
    try:
        return True, func(*args, **kwargs)
    except Exception as exc:
        return False, {
            "ok": False,
            "error": str(exc),
            "code": ERR_EXCEPTION,
        }


def safe_loop_step(state, step_func, on_error=None):
    ok, result = safe_call(step_func)
    if ok:
        return result

    error = result
    if state is not None and hasattr(state, "enter_safe"):
        state.enter_safe(error.get("error"))
    if on_error is not None:
        safe_call(on_error, error)
    return error


def before_detect(img, mode):
    return mode != "safe"


def after_detect(result):
    global _last_error, _miss_count
    if result is None:
        _miss_count += 1
        return result
    if result.get("ok"):
        _last_error = None
        _miss_count = 0
        return result
    _miss_count = result.get("miss", _miss_count + 1)
    _last_error = result.get("error")
    return result


def status():
    return {
        "last_error": _last_error,
        "miss_count": _miss_count,
    }


def reset():
    global _last_error, _miss_count
    _last_error = None
    _miss_count = 0
