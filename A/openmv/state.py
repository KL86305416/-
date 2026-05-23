MODE_IDLE = "idle"
MODE_RED = "red"
MODE_GREEN = "green"
MODE_BLUE = "blue"
MODE_COLOR = MODE_RED
MODE_LINE = "line"
MODE_SHAPE = "shape"
MODE_APRILTAG = "apriltag"
MODE_QRCODE = "qrcode"
MODE_AUTO = "auto"
MODE_SAFE = "safe"

MODES = (
    MODE_IDLE,
    MODE_RED,
    MODE_GREEN,
    MODE_BLUE,
    MODE_LINE,
    MODE_SHAPE,
    MODE_APRILTAG,
    MODE_QRCODE,
    MODE_AUTO,
    MODE_SAFE,
)


class VisionState:
    def __init__(self):
        self.mode = MODE_IDLE
        self.previous_mode = MODE_IDLE
        self.active = False
        self.debug = False
        self.safe = False
        self.save_requested = False
        self.last_error = None
        self.last_command = None
        self.last_result = None

    def reset(self):
        self.mode = MODE_IDLE
        self.previous_mode = MODE_IDLE
        self.active = False
        self.debug = False
        self.safe = False
        self.save_requested = False
        self.last_error = None
        self.last_command = None
        self.last_result = None

    def set_mode(self, mode):
        mode = self.normalize_mode(mode)
        if mode not in MODES or mode == MODE_SAFE:
            return False
        self.previous_mode = self.mode
        self.mode = mode
        self.safe = False
        return True

    def enter_safe(self, error=None):
        self.previous_mode = self.mode
        self.mode = MODE_SAFE
        self.active = False
        self.safe = True
        self.last_error = error

    def normalize_mode(self, mode):
        if mode is None:
            return ""
        mode = str(mode).strip().lower()
        if mode == "color":
            return MODE_RED
        if mode in ("tag", "april"):
            return MODE_APRILTAG
        if mode in ("qr", "code"):
            return MODE_QRCODE
        return mode

    def update(self, result):
        self.last_result = result
        if result and result.get("mode"):
            self.mode = self.normalize_mode(result.get("mode"))
        return result

    def handle_command(self, frame):
        if not frame:
            return self.reply("ERR", "EMPTY")
        fields = frame.get("fields", [])
        if not fields:
            return self.reply("ERR", "NO_CMD")

        cmd = str(fields[0]).strip().upper()
        args = fields[1:]
        self.last_command = cmd

        if cmd == "MODE":
            if not args:
                return self.reply("ERR", "NO_MODE")
            if not self.set_mode(args[0]):
                return self.reply("ERR", "BAD_MODE")
            return self.reply("OK", "MODE", self.mode)

        if cmd == "START":
            if self.mode == MODE_SAFE:
                return self.reply("ERR", "SAFE")
            self.active = True
            return self.reply("OK", "START", self.mode)

        if cmd == "STOP":
            self.active = False
            return self.reply("OK", "STOP", self.mode)

        if cmd == "DEBUG":
            self.debug = self._truthy(args[0]) if args else (not self.debug)
            return self.reply("OK", "DEBUG", 1 if self.debug else 0)

        if cmd == "RESET":
            self.reset()
            return self.reply("OK", "RESET")

        if cmd == "PING":
            return self.reply("PONG", self.mode, 1 if self.active else 0)

        if cmd == "SAVE":
            self.save_requested = True
            return self.reply("OK", "SAVE")

        return self.reply("ERR", "BAD_CMD")

    def reply(self, status, *values):
        return {
            "status": status,
            "values": values,
            "mode": self.mode,
            "active": self.active,
            "debug": self.debug,
            "safe": self.safe,
            "error": self.last_error,
        }

    def _truthy(self, value):
        return str(value).strip().upper() in ("1", "ON", "TRUE", "YES")


_STATE = VisionState()


def init():
    _STATE.reset()
    return _STATE


def reset():
    _STATE.reset()
    return _STATE


def update(result):
    return _STATE.update(result)


def set_result(result):
    return update(result)


def handle_command(frame):
    return _STATE.handle_command(frame)
