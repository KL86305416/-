# OpenMV 官方例程同步记录
- 日期：`2026-05-23`
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`implemented`
- 范围：`OpenMV Visual module(SJ)\code / backups / dev_logs / chat_data`

## 1. 本轮目标
把官方 OpenMV 例程实际拉取到本地工作区，避免当前目录里只有手写 `main.py` 而没有官方参考代码。

## 2. 官方来源
- 仓库：`https://github.com/openmv/openmv.git`
- 分支：`master`
- 提交：`e5b538651c591aeb7099fcf876e1b61e0e885d91`
- 同步目录：`scripts/examples`

## 3. 本地落盘结果
已同步到：

- `OpenMV Visual module(SJ)\code\examples\openmv_official`

同步结果：

- 文件总数：`430`
- 保留了官方原始目录层级
- 未覆盖现有 `OpenMV Visual module(SJ)\code\main.py`

## 4. 当前最相关示例
针对这条 `OpenMV N6 + 官方屏幕` 线，当前最值得直接参考的是：

- `OpenMV Visual module(SJ)\code\examples\openmv_official\50-OpenMV-Boards\53-N6-Boards\50-Board-Control\spi_control.py`
- `OpenMV Visual module(SJ)\code\examples\openmv_official\50-OpenMV-Boards\60-Shields\60-LCD-Shield\lcd_shield.py`
- `OpenMV Visual module(SJ)\code\examples\openmv_official\50-OpenMV-Boards\60-Shields\69-Touch-LCD-Shield\touch_lcd_shield.py`

## 5. 一并补充的说明文件
新增：

- `OpenMV Visual module(SJ)\code\examples\README.md`

更新：

- `OpenMV Visual module(SJ)\code\README.md`

## 6. 备份与归档
新增：

- `backups\2026-05-23_openmv_official_examples_sync\README.md`
- `backups\2026-05-23_openmv_official_examples_sync\main.py.snapshot`
- `backups\2026-05-23_openmv_official_examples_sync\README.md.snapshot`
- `chat_data\2026-05-23_phase_25_openmv_official_examples_sync.md`

## 7. 说明
这一步做的是“官方例程同步到工作区”，不是“把官方例程直接改成当前项目主入口”。当前主入口仍然是：

- `OpenMV Visual module(SJ)\code\main.py`

后续如果要继续推进，可以直接基于官方 `spi_control.py` 和现有 `main.py` 做一次针对你背板屏幕型号的合并整理。
