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
                                  id=data.get("id", data.get("code", 0)),
                                  x=data.get("x", 0), y=data.get("y", 0),
                                  w=data.get("w", 0), h=data.get("h", 0),
                                  score=data.get("score", data.get("pixels", 0)),
                                  area=data.get("area", 0),
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


def _thresholds(thresholds):
    if not thresholds:
        return []
    try:
        if len(thresholds) == 6 and not isinstance(thresholds[0], (tuple, list)):
            return [tuple(thresholds)]
    except TypeError:
        return []
    return thresholds


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


def detect_blobs(img, thresholds, roi=None, pixels_threshold=20,
                 area_threshold=20, merge=True, margin=5,
                 x_stride=2, y_stride=1, invert=False):
    thresholds = _thresholds(thresholds)
    roi = _roi(roi)
    if not thresholds:
        return []
    kwargs = {
        "pixels_threshold": pixels_threshold,
        "area_threshold": area_threshold,
        "merge": merge,
        "margin": margin,
        "x_stride": x_stride,
        "y_stride": y_stride,
        "invert": invert,
    }
    if roi is not None:
        kwargs["roi"] = roi
    try:
        return img.find_blobs(thresholds, **kwargs) or []
    except TypeError:
        if roi is not None:
            return img.find_blobs(thresholds, roi=roi, pixels_threshold=pixels_threshold,
                                  area_threshold=area_threshold, merge=merge) or []
        return img.find_blobs(thresholds, pixels_threshold=pixels_threshold,
                              area_threshold=area_threshold, merge=merge) or []


def largest_blob(blobs, key="pixels"):
    best = None
    best_score = -1
    for blob in blobs or []:
        score = _value(blob, key, None)
        if score is None and key != "area":
            score = _value(blob, "area", 0)
        if score is None:
            score = 0
        if best is None or score > best_score:
            best = blob
            best_score = score
    return best


def blob_to_result(blob, kind="blob", extra=None):
    if not blob:
        return _empty(kind)
    left = _value(blob, "x", 0)
    top = _value(blob, "y", 0)
    width = _value(blob, "w", 0)
    height = _value(blob, "h", 0)
    cx = _value(blob, "cx", left + width // 2)
    cy = _value(blob, "cy", top + height // 2)
    data = {
        "x": cx,
        "y": cy,
        "left": left,
        "top": top,
        "w": width,
        "h": height,
        "cx": cx,
        "cy": cy,
        "pixels": _value(blob, "pixels", 0),
        "area": _value(blob, "area", 0),
        "code": _value(blob, "code", 0),
        "rotation": _value(blob, "rotation", 0),
        "density": _value(blob, "density", 0),
        "elongation": _value(blob, "elongation", 0),
    }
    if extra:
        data.update(extra)
    return _result(kind, True, data)


def detect_color_target(img, thresholds, roi=None, pixels_threshold=20,
                        area_threshold=20, merge=True, margin=5):
    blobs = detect_blobs(img, thresholds, roi=roi,
                         pixels_threshold=pixels_threshold,
                         area_threshold=area_threshold,
                         merge=merge, margin=margin)
    blob = largest_blob(blobs)
    return blob_to_result(blob, "blob", {"count": len(blobs)})
