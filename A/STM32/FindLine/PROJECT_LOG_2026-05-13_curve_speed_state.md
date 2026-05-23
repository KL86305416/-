# 项目日志：新增弯道降速状态

- 时间：2026-05-13 13:16
- 改动范围：循迹控制速度策略、OLED 状态显示

## 问题目标

- 小车进入弯道时偏差较大，继续使用直道基础速度容易过冲。
- 需要在检测到弯道后降低整体速度，并把参数做成宏定义，便于后续调车。

## 改动内容

- 新增弯道状态 `curve_state_active`。
- 当滤波后的误差绝对值达到 `LINE_FOLLOWER_CURVE_ENTER_ERROR` 时进入弯道状态。
- 当误差绝对值降到 `LINE_FOLLOWER_CURVE_EXIT_ERROR` 以下时退出弯道状态，使用迟滞避免状态频繁跳变。
- 弯道状态下使用 `LINE_FOLLOWER_CURVE_BASE_SPEED_DUTY` 作为基础速度，再叠加原有按偏差降速逻辑。
- OLED 循迹页面新增 `LINE: CURVE` 状态，方便上车观察是否进入弯道状态。

## 新增参数

- `LINE_FOLLOWER_CURVE_ENTER_ERROR`：进入弯道状态的误差阈值，默认 `2.00f`。
- `LINE_FOLLOWER_CURVE_EXIT_ERROR`：退出弯道状态的误差阈值，默认 `1.20f`。
- `LINE_FOLLOWER_CURVE_BASE_SPEED_DUTY`：弯道基础速度，默认 `130`。

## 验证结果

- 已执行 `cmake --build build/Debug`，编译和链接通过。
- 构建结果：RAM 使用 `3232 B / 32 KB`，FLASH 使用 `21876 B / 128 KB`。
