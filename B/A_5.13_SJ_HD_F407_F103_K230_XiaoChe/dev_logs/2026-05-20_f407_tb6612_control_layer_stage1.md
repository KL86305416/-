# F407 TB6612 主线控制层阶段一记录
- 日期：`2026-05-20`
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`implemented`
- 范围：`F407 main control panel TB6612 / backups / dev_logs / chat_data`

## 1. 本轮目标
在不破坏现有 `F103 -> F407` 灰度短帧接收链和 OLED 监视页的前提下，把主线工程从“只有电机底层初始化”推进到“已经具备安全、可控、可验证的主控控制入口”。

本轮不直接上自动寻迹，而是先补齐最关键的上板验证能力：
1. 上电默认停机，避免主控一下载就误动作。
2. 提供最小的人工电机自检入口，先确认 TB6612 的 A/B 通道和正反方向。
3. 保持灰度监视继续在线，方便同步观察 F103 回传状态。

## 2. 本轮代码改动
### 2.1 `main.c`
在主线工程的 [main.c](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel%20TB6612/code/5.15_F407_TB6612/Core/Src/main.c) 中新增了一个轻量级 `VehicleControl` 骨架，当前包含：

- `VehicleControlState`
- `VehicleControl_Init()`
- `VehicleControl_Poll()`
- `VehicleControl_IsKeyPressed()`
- `VehicleControl_BuildMotorPercent()`
- `VehicleControl_ApplyMotorOutput()`
- `VehicleControl_UpdateLedState()`

当前按键定义为瞬时人工自检模式：
- `KEY_1`：`motor A` 正转
- `KEY_2`：`motor A` 反转
- `KEY_3`：`motor B` 正转
- `KEY_4`：`motor B` 反转

如果同一路同时按下正反两个键，则该路输出自动归零，避免冲突指令。

### 2.2 LED 状态指示
为避免左右还未最终映射时产生语义混淆，本轮 LED 先直接绑定到 `A/B` 电机通道，而不是强行写成“左轮/右轮”：

- `LED_1`：`motor A` 有非零输出时点亮
- `LED_2`：`motor B` 有非零输出时点亮

这样上板调试时可以快速看出当前到底是哪一路被主控命令拉起。

### 2.3 主循环挂接
主循环现已变为：
1. `GrayHost_Poll()`
2. `VehicleControl_Poll()`
3. `GrayMonitor_Poll(&g_gray_module)`

这意味着：
- F103 的短帧继续实时接收
- OLED 继续显示 103 侧回传的运行态数据
- TB6612 主线分支已经具备最小可执行的电机控制入口

### 2.4 `motor_drive.c`
顺手清理了 [motor_drive.c](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel%20TB6612/code/5.15_F407_TB6612/Core/Src/motor_drive.c) 中一个无意义的越界比较告警，使当前主线工程构建结果恢复为 `0 Error(s), 0 Warning(s)`。

## 3. 当前上板验证方式
本轮代码上板后，建议按下面顺序验证：

1. 上电后不按键，车应保持停机，`LED_1/LED_2` 熄灭。
2. 按住 `KEY_1`，只应有 `motor A` 以固定低速正转，`LED_1` 亮。
3. 按住 `KEY_2`，只应有 `motor A` 反转，`LED_1` 亮。
4. 按住 `KEY_3`，只应有 `motor B` 正转，`LED_2` 亮。
5. 按住 `KEY_4`，只应有 `motor B` 反转，`LED_2` 亮。
6. 释放按键后，对应通道应立即回零停机。

当前测试速度定义为：
- `VEHICLE_CONTROL_KEY_TEST_SPEED_PERCENT = 35`

这个速度有意保持偏低，目的是先验证方向和通道映射，而不是追求推力。

## 4. 构建验证
使用本机 Keil 命令行对以下工程完成重建：

- 工程：`F407 main control panel TB6612\code\5.15_F407_TB6612\MDK-ARM\5.15_F407_TB6612.uvprojx`
- 结果：`0 Error(s), 0 Warning(s)`
- 构建产物更新时间：`2026-05-20 17:42` 左右

说明当前这轮修改至少已经通过工程级编译检查，而不只是静态阅读通过。

## 5. 当前结论
到这一轮为止，F407/TB6612 主线已经从：
- “能收灰度数据”
- “能在 OLED 上显示”
- “电机底层函数已存在”

推进到了：
- “有安全停机默认态”
- “有可操作的人工电机自检入口”
- “可在不依赖自动寻迹的前提下独立验证 A/B 通道与正反方向”

这一步的意义很大，因为后续自动寻迹若出现跑偏，终于可以先排除“主控电机通道接错 / 方向写反 / 某一路根本没输出”这类基础问题。

## 6. 下一步建议
最自然的下一步是进入“实车映射确认”：

1. 把 `motor A / motor B` 对应到整车的左轮/右轮。
2. 记录“正百分比”在实车上对应的是前进还是后退。
3. 在确认左右和正反之后，再把 `GrayRuntimeFrame.scene + turn` 映射到左右轮差速输出。
4. 最后再做自动寻迹状态机，而不是现在就盲目闭环。
