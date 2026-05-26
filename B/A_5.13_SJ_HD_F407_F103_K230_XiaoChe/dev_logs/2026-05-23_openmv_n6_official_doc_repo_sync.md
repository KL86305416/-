# OpenMV N6 官方文档源码仓库同步记录
- 日期：`2026-05-23`
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`implemented`
- 范围：`OpenMV Visual module(SJ)\material\OpenMV_N6 / backups / dev_logs / chat_data`

## 1. 本轮目标
把 OpenMV 官方完整文档仓库体系中与 N6 相关的内容拉到本地，不再只依赖网页快照、PDF 和图片缓存。

## 2. 实际确认到的官方仓库结构
外层文档仓库不是 `openmv-docs`，而是：

- `https://github.com/openmv/openmv-doc.git`

该仓库本体主要负责文档工程外壳；真正文档内容在它的子模块中：

- `https://github.com/openmv/micropython-doc.git`

## 3. 本地同步结果
本地已落盘到：

- `OpenMV Visual module(SJ)\material\OpenMV_N6\official_doc_repo\openmv-doc`

当前提交为：

- 外层仓库：`5a5cc691947d08df71203a366bd4c48386474d45`
- 子模块仓库：`e596bf0ac5fe30d539c3f6f4808c0e49129f6efa`

同步后已确认：

- `git status` 正常
- `git submodule status` 正常

## 4. 一并补充的 N6 相关索引
新增：

- `OpenMV Visual module(SJ)\material\OpenMV_N6\official_doc_repo\README.md`
- `OpenMV Visual module(SJ)\material\OpenMV_N6\official_doc_repo\N6_related_paths.txt`

其中 `N6_related_paths.txt` 包含：

- 文档仓库中命中 `OpenMV N6 / STM32N6` 的文档路径
- `OPENMV_N6` 板级定义目录下的文件路径

## 5. 当前最关键的 N6 文档入口
- `openmv-doc\micropython\docs\openmvcam\quickref\openmv-n6.rst`
- `openmv-doc\micropython\docs\openmvcam\quickref.rst`
- `openmv-doc\micropython\docs\openmvcam\tutorial\io_tutorial.rst`
- `openmv-doc\micropython\docs\openmvcam\sensors\pag7936.rst`
- `openmv-doc\micropython\docs\openmvcam\sensors\ps5520.rst`
- `openmv-doc\micropython\ports\stm32\boards\OPENMV_N6`

## 6. 与此前资料目录的关系
此前目录里已有：

- 官方页面快照
- 官方 PDF
- 官方图片
- 中文整理文档

本轮新增的是：

- 官方文档源码仓库
- 子模块里的原始 `rst / po / 图片 / 板级定义` 内容

也就是说，当前 `OpenMV_N6` 资料目录已经同时具备：

- 面向查阅的快照资料
- 面向继续深挖的源码文档仓库

## 7. 说明
本轮没有本地构建 HTML 站点，只完成了官方文档源码同步与索引整理。
