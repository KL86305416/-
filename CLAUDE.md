# 304 电赛协作

## 自动同步说明（零手工）

已配置钩子：
- 每次对话结束 → CLAUDE.md 自动 commit + push 到 GitHub
- 每次发消息前 → 自动 git pull 拉队友更新

你不用管 Git，正常对话就行。

---

## 项目

电赛小车 + 机械臂。仓库：https://github.com/KL86305416/304

| 目录 | 内容 |
|------|------|
| A/ | 我方（OpenMV 视觉 + STM32 巡线） |
| B/ | 队友 |
| share/ | 共享 |
| stm32/ | 主控 |

## 硬件（别改）

- STM32F103C8T6，OpenMV（UART3, 115200），OLED（I2C1, 0x78），MPU6050
- 双电机 PWM：TIM1 CH1/CH4，GPIOB 12/13/14/15，范围 -7200~7200
- 按键 PA9/PA10/PA12/PA15（PULLUP，低有效）

## 已调参数（别改）

BALANCE_KP=336, KD=1.44, VELOCITY_KI=-0.0015
TURN_KP=5.0, KD=0.6
TRACK_KP_STRAIGHT=90, CURVE=120, KD=10

## 当前进度

- [x] 编译通过，OLED正常，按键响应，能跑
- [ ] PA12/PA15 按键不灵敏
- [ ] 启动无平衡阶段
- [ ] 电机方向待确认

## 操作记录

- 2026-05-26：精简 OpenMV 代码 → ai_slim.py
- 2026-05-26：配置 Git 自动同步钩子

## 行为规则

- 参数表不随便改
- 非必要不生成注释和文档
- 代码放工程目录，文档放 D:\ai
