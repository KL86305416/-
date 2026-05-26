# 阶段摘要 25：OpenMV 官方例程同步到本地
- 日期：`2026-05-23`
- 阶段编号：`25`
- 主题：`openmv_official_examples_sync`
- 范围：`OpenMV Visual module(SJ)\code / backups / dev_logs / chat_data`

## 1. 这一阶段为什么开始
上一阶段已经把 OpenMV N6 的实时取流显示脚本写到了本地，但当时本地代码目录里还没有完整的官方例程。用户随后明确要求把官方 OpenMV 例程拉到工作区，以便后续对照官方实现继续联调。

## 2. 这一阶段做了什么
- 访问官方 `openmv/openmv` 仓库
- 只提取 `scripts/examples` 目录
- 同步到 `OpenMV Visual module(SJ)\code\examples\openmv_official`
- 保留现有 `main.py` 和 `README.md`
- 新增本地来源说明文件，标明仓库地址与提交号

## 3. 本轮关键结果
当前 `OpenMV Visual module(SJ)\code` 不再只有手写脚本，还多了一套可直接查阅的官方例程：

- 官方例程总文件数：`430`
- 来源提交：`e5b538651c591aeb7099fcf876e1b61e0e885d91`

对当前硬件线最相关的例程是：

- `50-OpenMV-Boards/53-N6-Boards/50-Board-Control/spi_control.py`
- `50-OpenMV-Boards/60-Shields/60-LCD-Shield/lcd_shield.py`
- `50-OpenMV-Boards/60-Shields/69-Touch-LCD-Shield/touch_lcd_shield.py`

## 4. 当前状态
现在可以明确回答：

- 官方 OpenMV 例程已经拉到本地
- 自定义主脚本仍是 `OpenMV Visual module(SJ)\code\main.py`
- 官方例程作为参考库放在 `OpenMV Visual module(SJ)\code\examples\openmv_official`

## 5. 这一步之后最自然的下一步
如果继续推进，下一步最合理的是从两条线里选一条：

1. 直接试跑当前 `main.py`，确认背板屏幕能否稳定显示实时画面和左上角 FPS
2. 对照官方 `spi_control.py`，把你这块背板的实际屏参和引脚初始化再细化一轮
