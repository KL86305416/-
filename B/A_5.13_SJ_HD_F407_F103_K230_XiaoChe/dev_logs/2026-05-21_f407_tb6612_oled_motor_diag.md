# F407 TB6612 OLED 电机诊断层记录
- 日期：`2026-05-21`
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`implemented and build-verified`
- 范围：`F407 main control panel TB6612 / backups / dev_logs / chat_data`

## 1. 本轮目标
上一阶段已经把主线工程推进到“按键可直接驱动 motor A / motor B 做人工自检”。

但上板时仍然缺一个很关键的观察窗口：如果车不动，当前到底是
- 按键没读到，
- 百分比没下发，
- PWM 没变化，
- 还是 TB6612 方向脚状态不对。

本轮的目标不是上自动循迹，而是把这些诊断信息直接挂到现有 OLED 页面上，让板端排障不再只靠猜。

## 2. 本轮代码改动
### 2.1 `gray_monitor.h`
新增 `GrayMonitorDriveDiag` 结构体，对外暴露以下诊断字段：
- `motor_a_percent`
- `motor_b_percent`
- `key_1_pressed` ~ `key_4_pressed`
- `ain1_state / ain2_state / bin1_state / bin2_state`
- `pwm_a_compare / pwm_b_compare`

同时把 `GrayMonitor_Poll()` 接口改为：
- `GrayMonitor_Poll(const GrayModuleState *state, const GrayMonitorDriveDiag *drive_diag)`

### 2.2 `main.c`
把原先仅供 `main.c` 内部使用的按键自检状态，收敛到共享诊断结构 `g_drive_diag` 中。

新增：
- `VehicleControl_RefreshDiagnostics()`

它在每次应用电机输出后同步采样：
- `DJ_AIN1`
- `DJ_AIN2`
- `DJ_BIN1`
- `DJ_BIN2`
- `TIM2 CH3` compare
- `TIM1 CH4` compare

这样 OLED 读取到的就是“刚刚实际下发出去”的驱动状态，而不是一份脱离现场的推测值。

### 2.3 `gray_monitor.c`
OLED 页面从原来的单纯灰度监视，扩展为“F103 接收 + 驱动诊断”联合页，当前显示内容包括：
- 在线状态、数据年龄、4 个按键状态
- `turn` 与 `scene`
- `motor A / motor B` 的百分比与 PWM compare
- `AIN1/AIN2/BIN1/BIN2` 实时方向脚状态
- 精简后的灰度运行标志

这意味着按住某个按键时，可以直接从 OLED 看到：
- 输入有没有被识别
- 电机命令是不是非零
- PWM 有没有起来
- H 桥方向脚是不是对应到了预期组合

## 3. 构建验证
已使用本机 Keil 命令行直接重建：
- 可执行文件：`D:\New_Keil_32\UV4\UV4.exe`
- 工程：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe\F407 main control panel TB6612\code\5.15_F407_TB6612\MDK-ARM\5.15_F407_TB6612.uvprojx`
- 结果：`0 Error(s), 0 Warning(s)`
- 最新构建日志：`F407 main control panel TB6612\code\5.15_F407_TB6612\MDK-ARM\5.15_F407_TB6612\5.build_log.htm`
- 本次确认时间：`2026-05-21 20:16`

## 4. 当前意义
到这一步，主线 F407/TB6612 已经不只是“能按键拉电机”，而是进入了“上板时能直接定位电机链路卡在哪一层”的状态。

这对后续接入自动循迹很关键，因为一旦车跑偏或根本不动，可以先快速排除：
- 键值/输入层是否正常
- 电机百分比是否真的下发
- PWM 是否真的变化
- 方向控制脚是否按预期翻转

## 5. 下一步建议
最自然的下一步仍然是实车映射确认：

1. 逐个按下 `KEY_1 ~ KEY_4`，确认 `motor A / motor B` 分别对应整车哪一侧车轮。
2. 记录“正百分比”在实车上究竟对应前进还是后退。
3. 观察 OLED 上的 `% / PWM / IO` 是否与实际现象一致。
4. 在左右轮和正反方向都确认后，再把 `GrayRuntimeFrame.scene + turn` 映射到左右轮差速输出。

在这之前，不建议直接闭环接入自动循迹。
