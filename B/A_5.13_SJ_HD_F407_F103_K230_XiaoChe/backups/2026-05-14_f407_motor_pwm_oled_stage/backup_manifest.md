# F407 电机 PWM 与 OLED 监视页阶段备份清单

- 日期：2026-05-14
- 备份目录：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe\backups\2026-05-14_f407_motor_pwm_oled_stage`
- 目的：保留 `F407` 进入“电机 PWM 基础收正 + OLED 灰度监视页”阶段之前的关键文件快照

## 1. 本次备份范围

备份位置：
- `code_before_edit`

已备份文件：
- `main.c`
- `tim.c`
- `gray_module.c`
- `gray_module.h`
- `F407VET6_5.1_Test.uvprojx`

## 2. 对应改动阶段

该备份对应的阶段是：
- `F407` 从“已经能收 F103 短帧”推进到“电机 PWM 基础参数收正 + OLED 本地监视灰度运行态”

## 3. 建议用途

该备份可用于：
- 回看本轮之前 `F407` 的原始主控状态
- 对比 PWM 基础参数收正前后差异
- 对比 OLED 监视页接入前后差异
