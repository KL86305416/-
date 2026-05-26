# F407 DAP-Link 下载异常诊断
- 日期：2026-05-14
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 目标工程：`F407VET6_5.1_Test`
- 下载器：`DAP-Link / CMSIS-DAP`

## 1. 问题重新界定

本轮用户补充后，已确认当前下载异常对象是：

- `F407` 主控板
- 下载链路使用 `DAP-Link`
- `Keil` 弹窗为 `CMSIS-DAP - Cortex-M Error`
- 报错文案为 `RDDI-DAP Error`

这与前面讨论的：

- `F103`
- `ST-LINK`

不是同一条问题链路，不能直接复用前面的判断。

## 2. 先排除“程序把 SWD 关掉了”

检查以下文件：

- [stm32f4xx_hal_msp.c](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/Core/Src/stm32f4xx_hal_msp.c:1)
- [gpio.c](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/Core/Src/gpio.c:1)
- [main.c](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/Core/Src/main.c:1)

得到结论：

- 没有对 `PA13 / PA14` 做普通 GPIO 重配
- 没有 `JTAG/SWD` 关闭逻辑
- 没有 `IWDG / WWDG` 实际启用
- 没有 `Option Bytes / RDP / Flash OB` 操作
- 没有低功耗进入逻辑

因此当前代码不像是“主动把下载口封死”的类型。

## 3. 发现的第一处高风险：CMSIS-DAP 实际仍在 10MHz

检查：

- [F407VET6_5.1_Test.uvoptx](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/MDK-ARM/F407VET6_5.1_Test.uvoptx:1)

发现存在一个“表里不一”的现象：

- `<DbgClock>1000000</DbgClock>` 显示为 `1MHz`
- 但 `CMSIS_AGDI` 实际驱动串中仍是 `-TC10000000 -TT10000000`

也就是说，工程底层实际仍按：

- `10MHz`

在驱动 `CMSIS-DAP`。

对 `DAP-Link + STM32F4` 来说，这一设置偏激进，极容易带来：

- 连不上
- 偶发握手失败
- `RDDI-DAP Error`

## 4. 发现的第二处明确工程错误：HSE 常量与实板不符

检查：

- [system_stm32f4xx.c](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/Core/Src/system_stm32f4xx.c:50)
- [F407 main control panel.enet](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/PCB%20schematic%20diagram/F407%20main%20control%20panel.enet:1)

发现：

- 软件中 `HSE_VALUE = 25000000`
- 原理图/网表中的 `X1` 实际为 `8MHz`

这不是风格问题，而是明确不一致。

虽然 `PLL` 寄存器配置本身仍可能刚好让芯片跑在预期主频附近，但：

- `SystemCoreClock`
- `HAL_RCC_GetHCLKFreq()`
- `HAL_RCC_GetPCLKxFreq()`
- 基于这些频率计算出来的 `SysTick / UART / 延时`

都会受影响。

这类错误不一定直接造成 `DAP-Link` 完全失联，但它会显著增加：

- 固件一上电就运行异常
- 非复位方式附着调试困难

的概率。

## 5. 本轮已执行的收敛修改

### 5.1 已备份

备份目录：

- `backups\2026-05-14_f407_daplink_download_recovery`

已备份：

- `F407VET6_5.1_Test.uvoptx`
- `system_stm32f4xx.c`

### 5.2 已修改

1. 将 `CMSIS_AGDI` 的实际连接时钟从 `10MHz` 改为 `1MHz`
2. 将 `system_stm32f4xx.c` 中的 `HSE_VALUE` 从 `25MHz` 改为 `8MHz`

对应位置：

- [F407VET6_5.1_Test.uvoptx](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/MDK-ARM/F407VET6_5.1_Test.uvoptx:123)
- [system_stm32f4xx.c](D:/A_5.13_SJ_HD_F407_F103_K230_XiaoChe/F407%20main%20control%20panel/code/AAA_F407VET6_5.6_Test/Core/Src/system_stm32f4xx.c:50)

## 6. 当前最合理的判断

截至本轮，最合理的判断是：

- 这次 `F407` 下载异常不像业务代码造成
- 更像 `DAP-Link/CMSIS-DAP` 下载参数偏激进
- 同时工程里还存在一个真实的外部晶振常量错误

所以这轮问题的优先级应当是：

1. 先把下载链路参数收稳
2. 修正板级时钟常量错误
3. 再观察是否还存在纯硬件连接问题

## 7. 下一步建议

建议用户下一轮按这个顺序操作：

1. 重新打开 `Keil` 工程
2. 用当前修改后的工程直接再试一次连接/下载
3. 若仍报 `RDDI-DAP Error`
   - 手动把 `SWD` 时钟再降到 `100kHz`
   - 勾选 `Connect under Reset`
   - 前提是 `NRST` 也确实接到了 `DAP-Link`
4. 若仍失败
   - 仅保留 `SWDIO / SWCLK / GND / VTref / NRST`
   - 断开其余外设与模块
   - 再做最小系统连接验证

## 8. 特别提醒

对 `DAP-Link` 来说，以下两项非常关键：

- `VTref/3.3V` 必须接上，否则调试器无法正确判断目标电平
- `NRST` 如果没接上，`Connect under Reset` 基本无法发挥作用

因此如果后续继续失败，优先核实的不是代码，而是：

- `VTref`
- `NRST`
- `SWDIO`
- `SWCLK`
- `GND`

这五条链路是否完整可靠。

