# F407 本轮改动前备份清单

- 日期：2026-05-14
- 备份目录：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe\backups\2026-05-14_f407_uart_short_frame_rx_start`
- 目的：保留 `F407` 首轮接入 `F103 USART6` 运行态短帧之前的关键代码快照

## 1. 本次备份范围

备份位置：
- `code_before_edit`

已备份文件：
- `main.c`
- `gray_module.c`
- `gray_module.h`
- `vision_protocol.c`
- `vision_protocol.h`
- `usart.c`
- `usart.h`
- `stm32f4xx_it.c`
- `main.h`
- `F407VET6_5.1_Test.uvprojx`

## 2. 对应改动阶段

该备份对应的阶段是：
- `F407` 从“仅有协议骨架”进入“真正接收 `F103` 运行态短帧”的第一轮落地

## 3. 建议用途

该备份可用于：
- 回看本轮改动前 `F407` 的原始状态
- 与本轮接入后的 `USART6` 接收链做差异对照
- 日后主控控制层继续接入前做阶段回溯
