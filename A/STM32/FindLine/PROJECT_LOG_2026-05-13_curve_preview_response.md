# 项目日志：增加弯道预判与提前转向响应

- 时间：2026-05-13 17:00
- 改动范围：循迹弯道进入判断、转向修正步进

## 问题目标

- 小车接近弯道时，原逻辑要等滤波后的误差变大后才进入弯道状态，存在响应偏晚、容易冲出弯道的问题。

## 改动内容

- 新增 `LINE_FOLLOWER_CURVE_PREVIEW_ERROR`，用于弯道预判阈值。
- 新增 `LINE_FOLLOWER_CURVE_PREVIEW_GAIN`，用于根据误差变化趋势预测接近弯道的程度。
- 新增 `LINE_FOLLOWER_CURVE_MAX_CORRECTION_STEP_DUTY`，让接近弯道时每个控制周期允许更快地增加转向修正。
- 弯道进入条件从“仅看滤波误差”扩展为：
  - 滤波误差达到进入阈值；
  - 或原始偏差达到预判阈值；
  - 或预测偏差达到预判阈值。

## 默认参数

- `LINE_FOLLOWER_CURVE_PREVIEW_ERROR = 1.50f`
- `LINE_FOLLOWER_CURVE_PREVIEW_GAIN = 1.00f`
- `LINE_FOLLOWER_CURVE_MAX_CORRECTION_STEP_DUTY = 8.0f`

## 验证结果

- 已执行 `cmake --build build/Debug`，编译和链接通过。
- 构建结果：RAM 使用 `3304 B / 32 KB`，FLASH 使用 `22600 B / 128 KB`。
