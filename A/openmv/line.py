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
                                  score=data.get("score", data.get("pixels", 0)),
                                  area=data.get("area", 0),
                                  angle=data.get("angle", 0),
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


def _image_size(img):
    try:
        return img.width(), img.height()
    except TypeError:
        return img.width, img.height


def _find_blobs(img, thresholds, roi=None, pixels_threshold=20,
                area_threshold=20, merge=True):
    thresholds = _thresholds(thresholds)
    roi = _roi(roi)
    if not thresholds:
        return []
    kwargs = {
        "pixels_threshold": pixels_threshold,
        "area_threshold": area_threshold,
        "merge": merge,
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


def _largest(blobs):
    best = None
    best_pixels = -1
    for blob in blobs or []:
        pixels = _value(blob, "pixels", 0)
        if best is None or pixels > best_pixels:
            best = blob
            best_pixels = pixels
    return best


def _line_result(found, width, cx, cy, count, extra=None):
    if not found:
        return _empty("line")
    error = cx - (width // 2)
    data = {
        "x": cx,
        "y": cy,
        "error": error,
        "cx": cx,
        "cy": cy,
        "count": count,
        "center_x": width // 2,
    }
    if width:
        data["error_norm"] = error / (width / 2)
    else:
        data["error_norm"] = 0
    if extra:
        data.update(extra)
    return _result("line", True, data)


def line_follow_single(img, thresholds, roi=None, pixels_threshold=20,
                       area_threshold=20, merge=True):
    width, height = _image_size(img)
    if roi is None:
        roi = (0, height // 2, width, height // 2)
    blobs = _find_blobs(img, thresholds, roi=roi,
                        pixels_threshold=pixels_threshold,
                        area_threshold=area_threshold,
                        merge=merge)
    blob = _largest(blobs)
    if not blob:
        return _empty("line")
    return _line_result(True, width, _value(blob, "cx", 0), _value(blob, "cy", 0),
                        len(blobs), {
                            "left": _value(blob, "x", 0),
                            "top": _value(blob, "y", 0),
                            "w": _value(blob, "w", 0),
                            "h": _value(blob, "h", 0),
                            "pixels": _value(blob, "pixels", 0),
                            "mode": "single",
                        })


def line_follow_weighted(img, thresholds, rois=None, pixels_threshold=20,
                         area_threshold=20, merge=True):
    width, height = _image_size(img)
    if rois is None:
        band = height // 3
        rois = [
            (0, 0, width, band, 0.2),
            (0, band, width, band, 0.3),
            (0, band * 2, width, height - band * 2, 0.5),
        ]

    weighted_x = 0
    weighted_y = 0
    weight_sum = 0
    found = 0
    details = []

    for item in rois:
        try:
            if len(item) < 4:
                continue
            roi = item[:4]
            weight = item[4] if len(item) > 4 else 1
        except TypeError:
            continue
        if weight <= 0:
            continue
        blobs = _find_blobs(img, thresholds, roi=roi,
                            pixels_threshold=pixels_threshold,
                            area_threshold=area_threshold,
                            merge=merge)
        blob = _largest(blobs)
        if not blob:
            details.append({"roi": roi, "found": False})
            continue
        cx = _value(blob, "cx", 0)
        cy = _value(blob, "cy", 0)
        weighted_x += cx * weight
        weighted_y += cy * weight
        weight_sum += weight
        found += 1
        details.append({"roi": roi, "found": True, "cx": cx, "cy": cy,
                        "pixels": _value(blob, "pixels", 0), "weight": weight})

    if not found or not weight_sum:
        return _empty("line")
    cx = int(weighted_x / weight_sum)
    cy = int(weighted_y / weight_sum)
    return _line_result(True, width, cx, cy, found, {
        "mode": "weighted",
        "weight_sum": weight_sum,
        "rois": details,
    })
