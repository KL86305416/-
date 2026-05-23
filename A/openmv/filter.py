SMOOTH_KEYS = ("x", "y", "w", "h", "score", "area", "angle")


class ResultFilter:
    def __init__(self, alpha=0.5, hold_frames=3, lost_frames=10):
        self.alpha = alpha
        self.hold_frames = hold_frames
        self.lost_frames = lost_frames
        self.last = None
        self.miss = 0

    def reset(self):
        self.last = None
        self.miss = 0

    def update(self, result):
        if self._mode_changed(result):
            self.reset()

        if result and result.get("ok"):
            filtered = self._smooth(result)
            filtered["ok"] = True
            filtered["miss"] = 0
            self.last = filtered
            self.miss = 0
            return self._copy(filtered)

        self.miss += 1
        mode = result.get("mode") if result else (self.last.get("mode") if self.last else None)
        fps = result.get("fps", 0) if result else (self.last.get("fps", 0) if self.last else 0)

        if self.last is not None and self.miss <= self.hold_frames:
            held = self._copy(self.last)
            held["ok"] = True
            held["miss"] = self.miss
            return held

        if self.miss >= self.lost_frames:
            self.last = None
        return self.no_target(mode, self.miss, fps)

    def no_target(self, mode=None, miss=1, fps=0):
        return {
            "ok": False,
            "mode": mode,
            "id": 0,
            "x": 0,
            "y": 0,
            "w": 0,
            "h": 0,
            "score": 0,
            "area": 0,
            "angle": 0,
            "payload": None,
            "fps": fps,
            "miss": miss,
            "error": "no_target",
        }

    def _smooth(self, result):
        if self.last is None:
            return self._copy(result)

        filtered = self._copy(result)
        alpha = self.alpha
        beta = 1.0 - alpha
        for key in SMOOTH_KEYS:
            current = result.get(key)
            previous = self.last.get(key)
            if self._number(current) and self._number(previous):
                value = previous * beta + current * alpha
                if isinstance(current, int) and isinstance(previous, int):
                    value = int(value)
                filtered[key] = value
        return filtered

    def _mode_changed(self, result):
        if not result or self.last is None:
            return False
        mode = result.get("mode")
        last_mode = self.last.get("mode")
        return mode is not None and last_mode is not None and mode != last_mode

    def _copy(self, result):
        copied = {}
        for key in result:
            copied[key] = result[key]
        return copied

    def _number(self, value):
        return isinstance(value, (int, float))
