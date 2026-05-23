# 项目日志：直角弯丢线兜底转向

- 时间：2026-05-13
- 改动范围：直角弯识别阈值、丢线后的转向接管逻辑

## 问题现象

- 小车遇到直角弯时没有进入转向，而是继续向前冲出黑线后停车。
- 说明 2 cm 黑线在角点处可能只被传感器扫到一瞬间，原先需要连续 2 次确认，容易错过。

## 改动内容

- 将 `LINE_FOLLOWER_RIGHT_ANGLE_CONFIRM_COUNT` 从 `2U` 调整为 `1U`，提高窄线直角弯的触发速度。
- 将 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_MS` 从 `70U` 缩短到 `30U`，减少确认后继续向前压入的距离。
- 新增最近一次循迹方向、最近一次看到黑线时间、最近一次直角弯候选时间记录。
- 新增 `LineFollower_ShouldTurnAfterLineLost()`：
  - 如果刚刚看到过边缘/弯道迹象；
  - 或刚刚处于弯道状态；
  - 或上一帧误差已经明显偏边；
  - 突然丢线时不再停车，而是直接进入直角弯 `PIVOT` 转向。

## 验证结果

- 已执行 `cmake --build build/Debug`，编译和链接通过。
- 构建结果：RAM 使用 `3320 B / 32 KB`，FLASH 使用 `25052 B / 128 KB`。

## 注意事项

- 当前目录仍不是 Git 仓库，因此无法执行提交。
- 上车测试时，如果仍然冲出，优先继续减小 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_MS` 或增大 `LINE_FOLLOWER_RIGHT_ANGLE_OUTER_SPEED_DUTY`。
