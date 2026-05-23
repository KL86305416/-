try:
    from machine import UART
except ImportError:
    try:
        from pyb import UART
    except ImportError:
        UART = None

from protocol import PREFIX_COMMAND, PREFIX_VISION, build_frame, parse_frame


DEFAULT_UART_PORT = 3
DEFAULT_BAUDRATE = 115200


def init_uart(port=DEFAULT_UART_PORT, baudrate=DEFAULT_BAUDRATE, timeout_char=20):
    if UART is None:
        raise RuntimeError("UART is not available")
    try:
        return UART(port, baudrate, timeout_char=timeout_char)
    except TypeError:
        uart = UART(port, baudrate)
        if hasattr(uart, "init"):
            try:
                uart.init(baudrate, timeout_char=timeout_char)
            except TypeError:
                try:
                    uart.init(baudrate)
                except TypeError:
                    pass
        return uart


def _write(uart, frame):
    uart.write(frame)
    return frame


def _value(result, key, default=""):
    if result is None:
        return default
    return result.get(key, default)


def send_result(uart, result):
    return _write(uart, format_result(result))


def format_result(result):
    return build_frame(
        PREFIX_VISION,
        "RESULT",
        _value(result, "mode"),
        1 if _value(result, "ok", False) else 0,
        _value(result, "id", 0),
        _value(result, "x", 0),
        _value(result, "y", 0),
        _value(result, "w", 0),
        _value(result, "h", 0),
        _value(result, "score", 0),
        _value(result, "area", 0),
        _value(result, "angle", 0),
        _value(result, "payload", ""),
        _value(result, "fps", 0),
        _value(result, "miss", 0),
        _value(result, "error", ""),
    )


def send_heartbeat(uart, tick=0, mode="", active=0):
    return _write(uart, build_frame(PREFIX_VISION, "HB", tick, mode, active))


def send_error(uart, code, message=""):
    return _write(uart, build_frame(PREFIX_VISION, "ERR", code, message))


def read_command(uart):
    if uart is None or not uart.any():
        return None
    data = None
    if hasattr(uart, "readline"):
        data = uart.readline()
    if not data:
        data = uart.read()
    frame = parse_frame(data)
    if frame and frame.get("prefix") == PREFIX_COMMAND:
        return frame
    text = _decode(data)
    if text:
        return text.strip()
    return None


def _decode(data):
    if data is None:
        return ""
    if isinstance(data, str):
        return data
    try:
        return data.decode("utf-8")
    except Exception:
        return str(data)
