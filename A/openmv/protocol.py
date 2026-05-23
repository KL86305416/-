PREFIX_VISION = "MV"
PREFIX_COMMAND = "MC"
FRAME_START = "$"
CHECK_SEP = "*"
FIELD_SEP = ","

MV_RESULT_FIELDS = (
    "type",
    "mode",
    "ok",
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

MC_COMMAND_FIELDS = ("cmd", "args")


def _to_text(value):
    if value is None:
        return ""
    return str(value)


def xor_checksum(text):
    value = 0
    for ch in text:
        value ^= ord(ch)
    return value & 0xff


def checksum_hex(text):
    return "%02X" % xor_checksum(text)


def build_frame(prefix, *fields):
    prefix = _to_text(prefix).strip().upper()
    if prefix.startswith(FRAME_START):
        prefix = prefix[1:]
    body = prefix
    if fields:
        body += FIELD_SEP + FIELD_SEP.join([_to_text(v) for v in fields])
    return FRAME_START + body + CHECK_SEP + checksum_hex(body) + "\n"


def build_command(cmd, *args):
    return build_frame(PREFIX_COMMAND, _to_text(cmd).strip().upper(), *args)


def _value(data, key, default=""):
    if data is None:
        return default
    return data.get(key, default)


def build_result(result=None, *fields):
    if isinstance(result, dict):
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
    if result is None:
        return build_frame(PREFIX_VISION, "RESULT", *fields)
    return build_frame(PREFIX_VISION, "RESULT", result, *fields)


def parse_frame(data):
    if data is None:
        return None
    if isinstance(data, bytes):
        try:
            data = data.decode("utf-8")
        except UnicodeError:
            return None
    data = data.strip()
    if not data or data[0] != FRAME_START:
        return None

    star = data.rfind(CHECK_SEP)
    if star < 0:
        return None

    body = data[1:star]
    checksum = data[star + 1:star + 3]
    if len(checksum) != 2:
        return None
    try:
        received = int(checksum, 16)
    except ValueError:
        return None
    if xor_checksum(body) != received:
        return None

    parts = body.split(FIELD_SEP)
    if not parts:
        return None
    prefix = parts[0].upper()
    if prefix not in (PREFIX_VISION, PREFIX_COMMAND):
        return None

    fields = parts[1:]
    frame = {
        "prefix": prefix,
        "type": fields[0] if fields else "",
        "cmd": fields[0] if prefix == PREFIX_COMMAND and fields else "",
        "fields": fields,
        "args": fields[1:] if len(fields) > 1 else [],
        "body": body,
        "checksum": checksum.upper(),
    }
    if prefix == PREFIX_VISION and fields and fields[0] == "RESULT":
        frame["result"] = _result_fields(fields)
    return frame


def _result_fields(fields):
    result = {}
    for index, key in enumerate(MV_RESULT_FIELDS):
        result[key] = fields[index] if index < len(fields) else ""
    for key in ("ok", "id", "x", "y", "w", "h", "score", "area", "angle", "fps", "miss"):
        try:
            result[key] = int(result[key]) if result[key] != "" else 0
        except (TypeError, ValueError):
            pass
    result["ok"] = bool(result.get("ok"))
    return result
