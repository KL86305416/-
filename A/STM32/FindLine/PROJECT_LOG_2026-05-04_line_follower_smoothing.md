# 项目日志：巡线摆头抑制与速度参数优化

- 时间：2026-05-04 16:12
- 改动范围：巡线控制参数与控制算法

## 改动内容

- 将巡线基础速度从 240 降到 200。
- 将最大前进速度从 520 降到 420。
- 将转向增益从 80.0 降到 45.0，减少左右轮速度差。
- 新增中心死区 `LINE_FOLLOWER_CENTER_DEADBAND`，轻微偏差不立即转向。
- 新增误差低通滤波 `LINE_FOLLOWER_ERROR_FILTER_ALPHA`，减少传感器抖动带来的摆头。
- 新增转向修正步进限幅 `LINE_FOLLOWER_MAX_CORRECTION_STEP_DUTY`，避免车头突然猛打方向。

## 调参建议

- 车仍然摆头：继续降低 `LINE_FOLLOWER_TURN_GAIN_DUTY_PER_SENSOR`，例如 35.0。
- 车转弯跟不上：略微提高 `LINE_FOLLOWER_TURN_GAIN_DUTY_PER_SENSOR`，例如 55.0。
- 整体太快或太慢：优先修改 `LINE_FOLLOWER_BASE_SPEED_DUTY`。
- 动作仍然突兀：降低 `LINE_FOLLOWER_MAX_CORRECTION_STEP_DUTY`，例如 12.0。
