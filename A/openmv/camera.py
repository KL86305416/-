try:
    import sensor
except ImportError:
    sensor = None

try:
    from config import (
        CAMERA_PIXFORMAT,
        CAMERA_FRAMESIZE,
        CAMERA_WINDOWING,
        CAMERA_HMIRROR,
        CAMERA_VFLIP,
        CAMERA_SKIP_MS,
        CAMERA_AUTO_EXPOSURE,
        CAMERA_EXPOSURE_US,
        CAMERA_AUTO_GAIN,
        CAMERA_GAIN_DB,
        CAMERA_AUTO_WHITEBAL,
        CAMERA_RGB_GAIN_DB,
    )
except ImportError:
    CAMERA_PIXFORMAT = "RGB565"
    CAMERA_FRAMESIZE = "QVGA"
    CAMERA_WINDOWING = None
    CAMERA_HMIRROR = False
    CAMERA_VFLIP = False
    CAMERA_SKIP_MS = 1000
    CAMERA_AUTO_EXPOSURE = False
    CAMERA_EXPOSURE_US = 8000
    CAMERA_AUTO_GAIN = False
    CAMERA_GAIN_DB = 8
    CAMERA_AUTO_WHITEBAL = False
    CAMERA_RGB_GAIN_DB = (0, 0, 0)


_PIXFORMAT_MAP = {
    "RGB565": "RGB565",
    "GRAYSCALE": "GRAYSCALE",
    "GRAY": "GRAYSCALE",
    "BAYER": "BAYER",
    "JPEG": "JPEG",
}

_FRAMESIZE_MAP = {
    "QQQVGA": "QQQVGA",
    "QQVGA": "QQVGA",
    "QVGA": "QVGA",
    "VGA": "VGA",
}


def _require_sensor():
    if sensor is None:
        raise RuntimeError("OpenMV sensor module is not available")


def _sensor_const(name):
    _require_sensor()
    return getattr(sensor, name)


def _normalize_key(value, default):
    if value is None:
        return default
    return value.upper() if isinstance(value, str) else value


def _set_if_supported(name, *args):
    _require_sensor()
    func = getattr(sensor, name, None)
    if not func:
        return False
    try:
        func(*args)
        return True
    except Exception:
        return False


def _set_auto_value(name, enabled, keyword, value):
    _require_sensor()
    func = getattr(sensor, name)
    enabled = bool(enabled)
    if enabled or value is None:
        func(True)
        return
    try:
        func(False, **{keyword: value})
    except TypeError:
        func(False, value)


def resolve_pixformat(pixformat):
    key = _normalize_key(pixformat, "RGB565")
    name = _PIXFORMAT_MAP.get(key, key)
    return _sensor_const(name) if isinstance(name, str) else name


def resolve_framesize(framesize):
    key = _normalize_key(framesize, "QVGA")
    name = _FRAMESIZE_MAP.get(key, key)
    return _sensor_const(name) if isinstance(name, str) else name


def set_windowing_if_supported(windowing=None):
    if not windowing:
        return False
    return _set_if_supported("set_windowing", windowing)


def configure_exposure(auto_exposure=None, exposure_us=None):
    if auto_exposure is None:
        auto_exposure = CAMERA_AUTO_EXPOSURE
    if exposure_us is None:
        exposure_us = CAMERA_EXPOSURE_US
    _set_auto_value("set_auto_exposure", auto_exposure, "exposure_us", int(exposure_us) if exposure_us is not None else None)


def configure_gain(auto_gain=None, gain_db=None):
    if auto_gain is None:
        auto_gain = CAMERA_AUTO_GAIN
    if gain_db is None:
        gain_db = CAMERA_GAIN_DB
    _set_auto_value("set_auto_gain", auto_gain, "gain_db", float(gain_db) if gain_db is not None else None)


def configure_whitebal(auto_whitebal=None, rgb_gain_db=None):
    if auto_whitebal is None:
        auto_whitebal = CAMERA_AUTO_WHITEBAL
    if rgb_gain_db is None:
        rgb_gain_db = CAMERA_RGB_GAIN_DB
    _set_auto_value("set_auto_whitebal", auto_whitebal, "rgb_gain_db", rgb_gain_db)


def lock_camera_auto(exposure_us=None, gain_db=None, whitebal=None):
    configure_exposure(False if exposure_us is not None else True, exposure_us)
    configure_gain(False if gain_db is not None else True, gain_db)
    configure_whitebal(bool(whitebal) if whitebal is not None else CAMERA_AUTO_WHITEBAL)


def init_camera(
    pixformat=None,
    framesize=None,
    windowing=None,
    exposure_us=None,
    gain_db=None,
    whitebal=None,
    hmirror=None,
    vflip=None,
    skip_ms=None,
    auto_exposure=None,
    auto_gain=None,
    auto_whitebal=None,
    rgb_gain_db=None,
):
    _require_sensor()

    sensor.reset()
    sensor.set_pixformat(resolve_pixformat(pixformat or CAMERA_PIXFORMAT))
    sensor.set_framesize(resolve_framesize(framesize or CAMERA_FRAMESIZE))

    if hmirror is None:
        hmirror = CAMERA_HMIRROR
    if vflip is None:
        vflip = CAMERA_VFLIP
    _set_if_supported("set_hmirror", bool(hmirror))
    _set_if_supported("set_vflip", bool(vflip))
    set_windowing_if_supported(windowing if windowing is not None else CAMERA_WINDOWING)

    if whitebal is not None and auto_whitebal is None:
        auto_whitebal = bool(whitebal)
    configure_exposure(auto_exposure, exposure_us)
    configure_gain(auto_gain, gain_db)
    configure_whitebal(auto_whitebal, rgb_gain_db)

    wait_ms = CAMERA_SKIP_MS if skip_ms is None else skip_ms
    if wait_ms:
        sensor.skip_frames(time=int(wait_ms))
    return sensor


def init(*args, **kwargs):
    return init_camera(*args, **kwargs)


def setup(*args, **kwargs):
    return init_camera(*args, **kwargs)


def snapshot():
    _require_sensor()
    return sensor.snapshot()
