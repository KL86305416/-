# Debug switches. Keep all False for competition builds.
DEBUG = False
DEBUG_DRAW = False
DEBUG_FPS = False
DEBUG_SAVE_IMAGE = False
DEBUG_SAVE_INTERVAL_MS = 1000
DEBUG_IMAGE_PREFIX = "debug"

# Camera defaults for OpenMV.
CAMERA_PIXFORMAT = "RGB565"
CAMERA_FRAMESIZE = "QVGA"
CAMERA_WINDOWING = None
CAMERA_HMIRROR = False
CAMERA_VFLIP = False
CAMERA_SKIP_MS = 1000

# Auto flags use OpenMV semantics:
# True enables the camera's automatic algorithm, False locks to the value below.
CAMERA_AUTO_EXPOSURE = False
CAMERA_EXPOSURE_US = 8000
CAMERA_AUTO_GAIN = False
CAMERA_GAIN_DB = 8
CAMERA_AUTO_WHITEBAL = False
CAMERA_RGB_GAIN_DB = (0, 0, 0)

# Backward-compatible name used by older code.
CAMERA_WHITEBAL = CAMERA_AUTO_WHITEBAL

# Serial configuration.
UART_PORT = 3
UART_BAUDRATE = 115200
SEND_INTERVAL_MS = 50

# Modes.
MODE_IDLE = "idle"
MODE_RED = "red"
MODE_GREEN = "green"
MODE_BLUE = "blue"
MODE_LINE = "line"
MODE_SHAPE = "shape"
MODE_APRILTAG = "apriltag"
MODE_TAG = MODE_APRILTAG
MODE_QRCODE = "qrcode"
MODE_AUTO = "auto"
DEFAULT_MODE = MODE_AUTO

# LAB thresholds.
THRESHOLDS = {
    "red": (30, 100, 15, 127, 15, 127),
    "green": (25, 100, -70, -10, 5, 80),
    "blue": (20, 90, -20, 40, -90, -20),
    "black": (0, 35, -30, 30, -30, 30),
    "white": (70, 100, -20, 20, -20, 20),
}

# Regions of interest.
ROIS = {
    "full": (0, 0, 320, 240),
    "center": (80, 40, 160, 160),
    "upper": (0, 0, 320, 120),
    "lower": (0, 120, 320, 120),
    "line": (0, 80, 320, 120),
}

MODES = {
    MODE_IDLE: {"id": 0, "threshold": None, "roi": "full", "min_area": 0},
    MODE_RED: {"id": 1, "threshold": "red", "roi": "center", "min_area": 80},
    MODE_GREEN: {"id": 2, "threshold": "green", "roi": "center", "min_area": 80},
    MODE_BLUE: {"id": 3, "threshold": "blue", "roi": "center", "min_area": 80},
    MODE_LINE: {"id": 4, "threshold": "black", "roi": "line", "min_area": 120},
    MODE_SHAPE: {"id": 5, "threshold": None, "roi": "center", "min_area": 0},
    MODE_APRILTAG: {"id": 6, "threshold": None, "roi": "full", "min_area": 0},
    MODE_QRCODE: {"id": 7, "threshold": None, "roi": "full", "min_area": 0},
    MODE_AUTO: {"id": 9, "threshold": None, "roi": "full", "min_area": 80},
}

EXPOSURES = {
    "auto": {
        "auto_exposure": True,
        "exposure_us": None,
        "auto_gain": True,
        "gain_db": None,
        "auto_whitebal": True,
        "rgb_gain_db": None,
    },
    "normal": {
        "auto_exposure": False,
        "exposure_us": CAMERA_EXPOSURE_US,
        "auto_gain": False,
        "gain_db": CAMERA_GAIN_DB,
        "auto_whitebal": False,
        "rgb_gain_db": CAMERA_RGB_GAIN_DB,
    },
    "low_load": {
        "auto_exposure": False,
        "exposure_us": 6000,
        "auto_gain": False,
        "gain_db": 6,
        "auto_whitebal": False,
        "rgb_gain_db": CAMERA_RGB_GAIN_DB,
    },
}
