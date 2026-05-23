# 项目日志：延长直角弯识别后的前进距离

- 时间：2026-05-14
- 改动范围：直角弯 `ENTRY` 前进阶段参数

## 问题现象

- 实车观察发现，识别到直角弯后先前进再转向的策略更稳定。
- 当前小车已经会先前进，但前进距离仍略短，车身还没有完全压到合适位置就开始转弯。

## 改动内容

- 将 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_MS` 从 `70U` 增加到 `100U`。
- 保持 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_SPEED_DUTY = 95` 不变。
- 保持 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_CORRECTION_DUTY = 0` 不变，前进阶段继续尽量直走。
- 未改动直角弯识别阈值和转向力度，避免影响已经调好的响应速度。

## 验证结果

- 已执行 `cmake --build build/Debug`，编译和链接通过。
- 构建结果：RAM 使用 `3328 B / 32 KB`，FLASH 使用 `25504 B / 128 KB`。

## 调参建议

- 如果仍然偏短，可继续将 `LINE_FOLLOWER_RIGHT_ANGLE_ENTRY_MS` 增加到 `110U` 或 `120U`。
- 如果开始出现冲过角点，再回调到 `85U` 或 `90U`。
