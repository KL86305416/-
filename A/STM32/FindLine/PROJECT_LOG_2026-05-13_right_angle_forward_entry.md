# 项目日志：直角弯识别后先前进再转向

- 时间：2026-05-13
- 改动范围：直角弯进入阶段、立即转向策略

## 问题观察

- 上车观察发现，小车识别到直角弯后如果立刻转向，车身可能还没有到达角点中心，导致转弯姿态不稳定。
- 更稳的动作应当是：识别到直角后先低速向前走一小段，再执行原地/小半径转弯。

## 改动内容

- 取消“强边缘立即进入 `PIVOT`”的逻辑，只要识别时仍然能读到黑线，就统一先进入 `ENTRY`。
- 保留“已经丢线后的兜底 `PIVOT`”，因为此时已经没有黑线可继续压入。
- 将 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_MS` 从 `15U` 增加到 `60U`，让车头先向前压到角点附近。
- 将 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_SPEED_DUTY` 调整为 `105`，降低前进阶段速度。
- 将 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_CORRECTION_DUTY` 从 `65` 降到 `0`，前进阶段尽量直走，不提前打方向。

## 验证结果

- 已执行 `cmake --build build/Debug`，编译和链接通过。
- 构建结果：RAM 使用 `3328 B / 32 KB`，FLASH 使用 `25508 B / 128 KB`。

## 调参建议

- 如果仍然提前转，可继续增大 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_MS` 到 `70U` 或 `80U`。
- 如果前进后冲过角点，可降低到 `45U` 或 `50U`。
- 如果车身压入时偏得明显，可把 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_CORRECTION_DUTY` 微调到 `10` 或 `15`。
