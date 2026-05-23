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
                                  score=data.get("score", data.get("magnitude", 0)),
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


def _best(items, keys):
    best = None
    best_score = -1
    for item in items or []:
        score = 0
        for key in keys:
            if key == "area":
                value = _value(item, "w", 0) * _value(item, "h", 0)
            else:
                value = _value(item, key, 0)
            if value:
                score = value
                break
        if best is None or score > best_score:
            best = item
            best_score = score
    return best


def detect_circles(img, roi=None, threshold=2000, x_margin=10, y_margin=10,
                   r_margin=10, r_min=2, r_max=100, r_step=2):
    roi = _roi(roi)
    kwargs = {
        "threshold": threshold,
        "x_margin": x_margin,
        "y_margin": y_margin,
        "r_margin": r_margin,
        "r_min": r_min,
        "r_max": r_max,
        "r_step": r_step,
    }
    if roi is not None:
        kwargs["roi"] = roi
    try:
        circles = img.find_circles(**kwargs) or []
    except TypeError:
        if roi is not None:
            circles = img.find_circles(roi=roi, threshold=threshold) or []
        else:
            circles = img.find_circles(threshold=threshold) or []
    best = best_circle(circles)
    if best and circles and circles[0] is not best:
        circles.remove(best)
        circles.insert(0, best)
    return circles


def detect_rects(img, roi=None, threshold=10000):
    roi = _roi(roi)
    kwargs = {"threshold": threshold}
    if roi is not None:
        kwargs["roi"] = roi
    try:
        rects = img.find_rects(**kwargs) or []
    except TypeError:
        if roi is not None:
            rects = img.find_rects(roi=roi, threshold=threshold) or []
        else:
            rects = img.find_rects(threshold=threshold) or []
    best = best_rect(rects)
    if best and rects and rects[0] is not best:
        rects.remove(best)
        rects.insert(0, best)
    return rects


def circle_to_result(circle, kind="circle"):
    if not circle:
        return _empty(kind)
    x = _value(circle, "x", 0)
    y = _value(circle, "y", 0)
    r = _value(circle, "r", 0)
    data = {
        "x": x,
        "y": y,
        "left": x - r,
        "top": y - r,
        "w": r * 2,
        "h": r * 2,
        "r": r,
        "cx": x,
        "cy": y,
        "magnitude": _value(circle, "magnitude", 0),
        "area": r * r * 3,
    }
    return _result(kind, True, data)


def rect_to_result(rect, kind="rect"):
    if not rect:
        return _empty(kind)
    left = _value(rect, "x", 0)
    top = _value(rect, "y", 0)
    w = _value(rect, "w", 0)
    h = _value(rect, "h", 0)
    cx = left + w // 2
    cy = top + h // 2
    data = {
        "x": cx,
        "y": cy,
        "left": left,
        "top": top,
        "w": w,
        "h": h,
        "cx": cx,
        "cy": cy,
        "corners": _value(rect, "corners", None),
        "magnitude": _value(rect, "magnitude", 0),
    }
    return _result(kind, True, data)


def best_circle(circles):
    return _best(circles, ("magnitude", "r"))


def best_rect(rects):
    return _best(rects, ("magnitude", "area"))
