# 项目日志：巡线 S 形摆动抑制

- 时间：2026-05-04 16:20
- 改动范围：巡线控制算法与参数

## 问题现象

- 小车行进过程中摆头过快。
- 回正过于激进，导致 S 形前进。
- 摆动幅度会逐渐变大，容易丢线。

## 处理方案

- 将基础速度降到 `160`，最高速度限制为 `320`。
- 将单纯比例转向改为带阻尼的 PD 控制。
- 新增最大转向修正限制 `LINE_FOLLOWER_MAX_CORRECTION_DUTY`。
- 将每个控制周期的转向变化限制为 `LINE_FOLLOWER_MAX_CORRECTION_STEP_DUTY`。
- 偏差越大，小车自动略微降速，降低弯道和回正时的惯性。
- 重新捕线时清零微分突变，避免刚找回线就猛摆。

## 后续调参方向

- 仍然 S 形：降低 `LINE_FOLLOWER_PROPORTIONAL_GAIN`，例如 `20.0f`。
- 回正仍然太猛：降低 `LINE_FOLLOWER_MAX_CORRECTION_STEP_DUTY`，例如 `5.0f`。
- 弯道跟不上：略微提高 `LINE_FOLLOWER_MAX_CORRECTION_DUTY`，例如 `100.0f`。
- 整体过慢：提高 `LINE_FOLLOWER_BASE_SPEED_DUTY`，例如 `180`。
