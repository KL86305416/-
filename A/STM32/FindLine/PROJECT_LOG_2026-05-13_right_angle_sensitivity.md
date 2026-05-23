# 项目日志：提升直角弯识别灵敏度

- 时间：2026-05-13
- 改动范围：直角弯候选识别、丢线兜底时间、转向速度参数

## 问题现象

- 小车已经能够在部分直角弯处转弯，但识别窗口仍偏窄。
- 角点处如果只扫到一瞬间的次外侧黑线，仍可能错过识别并冲出赛道。

## 改动内容

- 新增 `LINE_SENSOR_NEAR_OUTER_MASK`，将次外侧传感器 `0x02/0x40` 纳入直角弯候选识别。
- 新增 `LINE_FOLLOWER_RIGHT_ANGLE_NEAR_EDGE_ERROR`，用于识别次外侧单点或少量传感器触发的窄线角点。
- 放宽一侧多点分支识别：
  - `LINE_FOLLOWER_RIGHT_ANGLE_WIDE_ERROR` 从 `1.25f` 降低到 `0.85f`；
  - `LINE_FOLLOWER_RIGHT_ANGLE_WIDE_COUNT` 从 `3U` 降低到 `2U`。
- 新增 `LineFollower_ShouldPivotImmediately()`：
  - 如果黑线已在外侧/次外侧且中心传感器没有线，直接进入 `PIVOT` 转向；
  - 减少继续向前压入导致冲出赛道的距离。
- 将丢线兜底窗口从 `160 ms` 增加到 `240 ms`，并将丢线触发误差从 `1.20f` 降到 `0.65f`。
- 将直角转向外侧轮速度提高到 `170`，内侧轮反转速度提高到 `-120`，让转向动作更快建立。

## 验证结果

- 已执行 `cmake --build build/Debug`，编译和链接通过。
- 构建结果：RAM 使用 `3320 B / 32 KB`，FLASH 使用 `25360 B / 128 KB`。

## 注意事项

- 当前目录不是 Git 仓库，无法进行本次功能提交。
- 上车测试时如果出现误判过早转弯，优先把 `LINE_FOLLOWER_RIGHT_ANGLE_WIDE_ERROR` 从 `0.85f` 回调到 `1.00f`。
