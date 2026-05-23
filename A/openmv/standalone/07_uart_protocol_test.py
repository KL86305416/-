import time

try:
    from machine import UART
except ImportError:
    from pyb import UART


UART_PORT = 3
UART_BAUDRATE = 115200


def checksum_hex(body):
    value = 0
    for ch in body:
        value ^= ord(ch)
    return "%02X" % (value & 0xff)


def build_frame(prefix, *fields):
    body = prefix
    for field in fields:
        body += "," + str(field)
    return "$" + body + "*" + checksum_hex(body) + "\n"


def parse_frame(data):
    if not data:
        return None
    if not isinstance(data, str):
        try:
            data = data.decode()
        except Exception:
            data = str(data)
    data = data.strip()
    if not data or data[0] != "$":
        return None
    star = data.rfind("*")
    if star < 0:
        return None
    body = data[1:star]
    cs = data[star + 1:star + 3]
    if checksum_hex(body) != cs.upper():
        return None
    return body.split(",")


uart = UART(UART_PORT, UART_BAUDRATE, timeout_char=20)
mode = "idle"
tick = 0

while True:
    if uart.any():
        data = uart.readline()
        frame = parse_frame(data)
        if frame and len(frame) >= 3 and frame[0] == "MC" and frame[1] == "MODE":
            mode = frame[2]
            uart.write(build_frame("MV", "ACK", "MODE", mode))
        elif frame and len(frame) >= 2 and frame[0] == "MC" and frame[1] == "STOP":
            mode = "idle"
            uart.write(build_frame("MV", "ACK", "STOP", mode))
        else:
            uart.write(build_frame("MV", "ERR", "BAD_CMD"))

    uart.write(build_frame("MV", "HB", tick, mode))
    print("tick=%d mode=%s" % (tick, mode))
    tick += 1
    time.sleep_ms(500)
