try:
    from result import make_result as _make_result
    from result import no_target as _no_target
except Exception:
    _make_result = None
    _no_target = None


def _result(kind, found, data=None):
    if data is None:
        data = {}
    if _make_result:
        try:
            result = _make_result(ok=found, mode=kind,
                                  id=data.get("id", 0),
                                  x=data.get("x", 0), y=data.get("y", 0),
                                  w=data.get("w", 0), h=data.get("h", 0),
                                  score=data.get("score", data.get("decision_margin", 0)),
                                  area=data.get("area", data.get("w", 0) * data.get("h", 0)),
                                  angle=data.get("angle", data.get("rotation", 0)),
                                  payload=data.get("payload", None),
                                  error=data.get("error", None))
            result.update(data)
            return result
        except TypeError:
            try:
                result = _make_result(found, kind)
                result.update(data)
                return result
            except TypeError:
                pass
    data["type"] = kind
    data["found"] = found
    data["ok"] = found
    return data


def _empty(kind):
    if _no_target:
        try:
            result = _no_target(kind)
            if result is not None:
                return result
        except TypeError:
            try:
                result = _no_target(mode=kind)
                if result is not None:
                    return result
            except TypeError:
                pass
    return _result(kind, False, {})


def _value(obj, name, default=None):
    attr = getattr(obj, name, None)
    if attr is None:
        return default
    try:
        return attr()
    except TypeError:
        return attr


def _roi(roi):
    try:
        if roi is None or len(roi) < 4:
            return None
        x, y, w, h = roi[:4]
    except TypeError:
        return None
    if w <= 0 or h <= 0:
        return None
    return (int(x), int(y), int(w), int(h))


def _area(obj):
    return _value(obj, "w", 0) * _value(obj, "h", 0)


def _sort_targets(items, key):
    items = list(items or [])
    try:
        items.sort(key=key, reverse=True)
    except Exception:
        pass
    return items


def detect_apriltags(img, roi=None, families=None, fx=None, fy=None,
                     cx=None, cy=None):
    roi = _roi(roi)
    kwargs = {}
    if roi is not None:
        kwargs["roi"] = roi
    if families is not None:
        kwargs["families"] = families
    if fx is not None:
        kwargs["fx"] = fx
    if fy is not None:
        kwargs["fy"] = fy
    if cx is not None:
        kwargs["cx"] = cx
    if cy is not None:
        kwargs["cy"] = cy
    try:
        return _sort_targets(img.find_apriltags(**kwargs), _tag_score)
    except TypeError:
        if roi is not None:
            return _sort_targets(img.find_apriltags(roi=roi), _tag_score)
        return _sort_targets(img.find_apriltags(), _tag_score)


def detect_qrcodes(img, roi=None):
    roi = _roi(roi)
    try:
        if roi is not None:
            return _sort_targets(img.find_qrcodes(roi=roi), _area)
        return _sort_targets(img.find_qrcodes(), _area)
    except TypeError:
        return _sort_targets(img.find_qrcodes(), _area)


def _tag_score(tag):
    score = _value(tag, "decision_margin", 0)
    if score:
        return score
    return _area(tag)


def tag_to_result(tag, kind="apriltag"):
    if not tag:
        return _empty(kind)
    left = _value(tag, "x", 0)
    top = _value(tag, "y", 0)
    width = _value(tag, "w", 0)
    height = _value(tag, "h", 0)
    cx = _value(tag, "cx", left + width // 2)
    cy = _value(tag, "cy", top + height // 2)
    data = {
        "id": _value(tag, "id", 0),
        "family": _value(tag, "family", 0),
        "x": cx,
        "y": cy,
        "left": left,
        "top": top,
        "w": width,
        "h": height,
        "cx": cx,
        "cy": cy,
        "rotation": _value(tag, "rotation", 0),
        "decision_margin": _value(tag, "decision_margin", 0),
        "hamming": _value(tag, "hamming", 0),
        "x_translation": _value(tag, "x_translation", 0),
        "y_translation": _value(tag, "y_translation", 0),
        "z_translation": _value(tag, "z_translation", 0),
        "x_rotation": _value(tag, "x_rotation", 0),
        "y_rotation": _value(tag, "y_rotation", 0),
        "z_rotation": _value(tag, "z_rotation", 0),
    }
    return _result(kind, True, data)


def qrcode_to_result(code, kind="qrcode"):
    if not code:
        return _empty(kind)
    payload = _value(code, "payload", "")
    left = _value(code, "x", 0)
    top = _value(code, "y", 0)
    width = _value(code, "w", 0)
    height = _value(code, "h", 0)
    cx = _value(code, "cx", left + width // 2)
    cy = _value(code, "cy", top + height // 2)
    data = {
        "payload": payload,
        "text": payload,
        "x": cx,
        "y": cy,
        "left": left,
        "top": top,
        "w": width,
        "h": height,
        "cx": cx,
        "cy": cy,
        "corners": _value(code, "corners", None),
        "version": _value(code, "version", 0),
        "ecc_level": _value(code, "ecc_level", 0),
        "mask": _value(code, "mask", 0),
    }
    return _result(kind, True, data)
