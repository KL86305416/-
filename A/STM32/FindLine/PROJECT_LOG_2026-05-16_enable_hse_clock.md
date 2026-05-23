# 2026-05-16 启用外部高速晶振

## 目标

- 启用板载外部高速晶振 HSE。
- 保持系统主频仍为 16MHz，避免影响现有 I2C、USART、PWM 和延时参数。

## 改动

- 修改 `SystemClock_Config()`：
  - 时钟源从 HSI 改为 HSE。
  - HSE 8MHz 作为 PLL 输入。
  - PLL 参数设置为 `M=1`、`N=16`、`R=8`，输出 16MHz SYSCLK。
  - 系统时钟源改为 `RCC_SYSCLKSOURCE_PLLCLK`。

## 时钟路径

```text
HSE 8MHz -> PLL /1 *16 /8 -> SYSCLK 16MHz
```

## 验证

- 已执行 `cmake --build build\Debug`。
- 编译通过，生成 `FindLine.elf`。

## 注意

- 如果外部晶振未焊接、损坏或负载电容不匹配，启动会进入 `Error_Handler()`。
- 当前工程目录不是 Git 仓库，无法执行本次功能提交。
