# F407 TB6612 分支 USART6 短帧接收与 OLED 监视迁移
- 日期：2026-05-15
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`implemented`
- 范围：`F407 main control panel TB6612 / backups / dev_logs / chat_data`

## 1. 本次目标

在不提前介入 `TB6612` 电机驱动闭环的前提下，先把 `TB6612` 分支补到“能看见 F103 运行态数据”的状态。

本次只处理两件事：

1. 接回 `F103 -> F407` 的 `USART6` 运行态短帧接收
2. 在 `I2C OLED` 上显示 `F103` 回传的关键状态

## 2. 本次新增与修改

### 2.1 新增文件

- `Core/Inc/gray_module.h`
- `Core/Src/gray_module.c`
- `Core/Inc/gray_monitor.h`
- `Core/Src/gray_monitor.c`
- `Core/Inc/oled_display.h`
- `Core/Src/oled_display.c`

### 2.2 修改文件

- `Core/Src/main.c`
- `MDK-ARM/5.15_F407_TB6612.uvprojx`

### 2.3 备份

本次改动前已备份：

- `backups/2026-05-15_f407_tb6612_uart_oled_migration/code_before_edit/main.c`
- `backups/2026-05-15_f407_tb6612_uart_oled_migration/code_before_edit/5.15_F407_TB6612.uvprojx`

## 3. 实际接回的链路

### 3.1 串口接收链路

当前接回的是：

- `USART6`
- 单字节中断接收
- 本地状态机拼包
- 解析 `AA 55 + turn + scene + flags + crc8`

在主循环中：

- `GrayHost_Poll()` 负责在线状态维护
- `HAL_UART_RxCpltCallback()` 负责逐字节喂入解析器
- `HAL_UART_ErrorCallback()` 负责在串口异常后重新挂接接收

这样做的目的，是先把最小可用的数据接入链路恢复出来，而不是一上来就把更多调试逻辑堆进去。

### 3.2 运行态短帧结构

当前主控侧按既有方案解析 6 字节运行态短帧：

1. `0xAA`
2. `0x55`
3. `turn`
4. `scene`
5. `flags`
6. `crc8`

这与前面已经确定的“103 本地尽量多做判断、407 尽量接收短结论帧”的方向一致。

## 4. OLED 监视页

### 4.1 采用的显示通道

本次沿用板上的 `I2C1 OLED` 通道：

- `PB6` -> `I2C_OLED_SCL`
- `PB7` -> `I2C_OLED_SDA`

OLED 驱动按 `SSD1306` 兼容方式处理，启动时优先尝试：

- `0x78`
- `0x7A`

### 4.2 当前显示内容

当前 OLED 监视页会循环显示：

- 在线状态与数据年龄
- `turn`
- `scene`
- `flags`
- `LINE_FOUND / CENTER_ON_LINE`
- `LEFT_FEATURE / RIGHT_FEATURE / WIDE_BLACK`
- `UNSTABLE / SENSOR_FAULT / VALID`

这意味着后面即使还没把 `TB6612` 电机控制完全接上，也已经可以在主控板上直接判断：

- `F103` 是否在持续发数据
- 当前建议左转还是右转
- 当前路况被判断成什么场景
- 标志位有没有异常抖动

## 5. `main.c` 当前状态变化

`TB6612` 分支的 `main.c` 已经从纯骨架变成了“带最小业务初始化”的版本。

当前启动后会做：

1. 完成原有 CubeMX 外设初始化
2. 初始化 `GrayModuleState`
3. 初始化 `GrayMonitor`
4. 启动 `USART6` 单字节中断接收

主循环当前会做：

1. 刷新在线状态
2. 刷新 OLED 监视页

这标志着 `TB6612` 分支第一次真正接回了主控侧的运行态业务链路。

## 6. 工程文件同步

本次不仅新增了源文件，还同步修改了 `MDK-ARM/5.15_F407_TB6612.uvprojx`，把以下源文件挂入 Keil 工程：

- `gray_module.c`
- `oled_display.c`
- `gray_monitor.c`

这样在你后面直接用 Keil 打开工程时，这些文件会参与编译，而不是只存在于磁盘里。

## 7. 校验结果

本次完成了一轮本地语法校验：

- 工具：本机 `gcc`
- 方式：`-fsyntax-only`
- 结果：通过

出现的告警为 CMSIS 在宿主机 `x64 gcc` 下常见的指针宽度 warning，不属于本次新增业务代码错误。

本次没有完成的校验：

- 没有做 Keil 完整构建
- 没有做实板下载验证
- 没有做 OLED 实际点亮确认

## 8. 当前仍未处理的部分

本次刻意没有进入以下内容：

- `TB6612` 电机驱动层
- `PA15 / DJ_BIN2` GPIO 模式修正
- `TIM1 / TIM2` PWM 频率整定
- 车辆运动闭环

原因很明确：

- 先恢复“看见数据”
- 再处理“驱动执行”

这会比同时改通信、显示、电机三条链更稳。

## 9. 建议的下一步

下一步最合理的是：

1. 实板确认 `USART6` 是否已经正常收到 `F103` 运行态短帧
2. 实板确认 `I2C OLED` 是否正常显示数据
3. 确认无误后，再进入 `TB6612` 电机底层适配：
   - `PA15` 改成方向输出
   - `TIM1/TIM2` 调到明确 PWM 频率
   - 新建 `TB6612` 版 `motor_drive`

如果这一步板上现象正常，就说明 `TB6612` 分支已经从“硬件空壳”进入了“主控业务重新接回”的阶段。
