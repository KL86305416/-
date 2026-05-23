# 项目日志：丢线后边缘再捕获优化

- 时间：2026-05-04 16:43
- 改动范围：巡线丢线恢复状态机

## 问题现象

- 全白丢线后，小车可以先往反方向纠正。
- 但一侧传感器刚接触黑线时，程序马上切回普通巡线。
- 普通巡线会再次把车推向丢线方向，导致“丢线、纠正、再丢线”循环。

## 改动内容

- 新增“再捕获阶段”：丢线恢复过程中，即使边缘传感器读到黑线，也暂不立刻恢复普通巡线。
- 黑线需要进入中间区域，且连续稳定若干控制周期，才退出丢线恢复。
- 新增 `LINE_FOLLOWER_REACQUIRE_SPEED_DUTY`，控制边缘再捕获速度。
- 新增 `LINE_FOLLOWER_REACQUIRE_CENTER_THRESHOLD`，控制认为“回到中间”的阈值。
- 新增 `LINE_FOLLOWER_REACQUIRE_STABLE_CYCLES`，控制退出恢复模式前的稳定采样次数。

## 调参建议

- 仍然反复丢线：增大 `LINE_FOLLOWER_REACQUIRE_CENTER_THRESHOLD` 或 `LINE_FOLLOWER_REACQUIRE_STABLE_CYCLES`。
- 再捕获太慢：提高 `LINE_FOLLOWER_REACQUIRE_SPEED_DUTY`。
- 再捕获时又冲过头：降低 `LINE_FOLLOWER_REACQUIRE_SPEED_DUTY`。
