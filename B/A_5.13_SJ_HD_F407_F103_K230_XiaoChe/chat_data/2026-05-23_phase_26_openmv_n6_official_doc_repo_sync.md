# 阶段摘要 26：OpenMV N6 官方文档源码仓库同步到本地
- 日期：`2026-05-23`
- 阶段编号：`26`
- 主题：`openmv_n6_official_doc_repo_sync`
- 范围：`OpenMV Visual module(SJ)\material\OpenMV_N6 / backups / dev_logs / chat_data`

## 1. 这一阶段为什么开始
在完成 OpenMV 官方例程同步后，用户继续要求把“完整文档仓库中关于 N6 的内容”拉到本地。此前本地虽然已经有网页快照、PDF 和图片，但还没有官方文档源码仓库本身。

## 2. 这一阶段实际确认到的结构
排查后确认：

- 不存在 `openmv-docs` 这个官方仓库
- OpenMV 官方文档外层仓库是 `openmv/openmv-doc`
- 真正文档内容由它挂载的子模块 `openmv/micropython-doc` 提供

## 3. 这一阶段做了什么
- 拉取 `openmv/openmv-doc`
- 初始化并同步其 `micropython` 子模块
- 把整套仓库保留为本地可继续更新的 Git 仓库
- 生成 `N6_related_paths.txt` 作为 N6 相关入口索引
- 给现有中文整理文档补上源码仓库入口说明

## 4. 当前结果
本地现在新增了：

- `OpenMV Visual module(SJ)\material\OpenMV_N6\official_doc_repo\openmv-doc`

对应提交：

- `openmv-doc`：`5a5cc691947d08df71203a366bd4c48386474d45`
- `micropython-doc`：`e596bf0ac5fe30d539c3f6f4808c0e49129f6efa`

也就是说，当前 `OpenMV_N6` 资料目录已经不只是“快照资料”，而是多了一套官方文档源码仓库本体。

## 5. 对当前任务最有用的入口
后续如果继续围绕 `OpenMV N6 + 官方屏幕 + 接口联调` 推进，优先看的路径是：

- `openmv-doc\micropython\docs\openmvcam\quickref\openmv-n6.rst`
- `openmv-doc\micropython\docs\openmvcam\tutorial\io_tutorial.rst`
- `openmv-doc\micropython\docs\openmvcam\sensors\pag7936.rst`
- `openmv-doc\micropython\ports\stm32\boards\OPENMV_N6`

## 6. 这一步之后最自然的下一步
如果继续推进，下一步最合理的是二选一：

1. 把 `openmv-n6.rst` 和你当前本地快照资料合并，单独整理一份“只保留 N6 接口与屏幕相关信息”的中文摘要
2. 直接从 `OPENMV_N6` 板级定义和 `spi_control.py` 两条线入手，继续收敛你那块屏幕背板的实际初始化参数
