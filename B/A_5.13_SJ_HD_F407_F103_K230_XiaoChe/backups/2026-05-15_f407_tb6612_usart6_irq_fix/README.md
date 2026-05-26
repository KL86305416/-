# F407 TB6612 USART6 IRQ Fix Backup

- Date: `2026-05-15`
- Workspace: `D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- Scope: `F407 main control panel TB6612\code\5.15_F407_TB6612`
- Purpose: backup the key files before restoring the `USART6` interrupt receive chain

## Backed-up files

- `code_before_fix\usart.c`
- `code_before_fix\stm32f4xx_it.c`
- `code_before_fix\stm32f4xx_it.h`

## Why this backup exists

The migrated TB6612 branch started `HAL_UART_Receive_IT()` for `USART6`, but the actual interrupt path was incomplete:

- `USART6_IRQn` was not enabled in `HAL_UART_MspInit()`
- `USART6_IRQHandler()` was missing in `stm32f4xx_it.c`
- `USART6_IRQHandler()` prototype was missing in `stm32f4xx_it.h`

With that state, bytes from the F103 grayscale board could not enter the runtime frame parser even if wiring and baud rate were correct.

## Fix applied after this backup

- enable `USART6_IRQn`
- add `USART6_IRQHandler()` and forward to `HAL_UART_IRQHandler(&huart6)`
- declare `USART6_IRQHandler()` in the interrupt header
