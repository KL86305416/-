# 步进云台 TB6612 工程目录重构记录
- 日期：`2026-05-20`
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`staged_for_commit`
- 范围：`F407 main control panel TB6612 / F407 main control panel TB6612 Stepper motor pan-tilt version / dev_logs / chat_data`

## 1. 触发背景

当前仓库中原先存在一套步进云台版本的 `TB6612` 工程，路径为：

```text
F407 main control panel TB6612/Stepper motor pan-tilt version code/5.15_F407_TB6612
```

本轮用户已将这套工程做目录层级重构，目标是把它从原先嵌套在 `F407 main control panel TB6612` 下的旧位置，搬到工作区根目录下的新独立目录。

## 2. 本轮确认出的核心事实

本轮核对后，确认这次更新的核心是：

1. 原先旧路径下的整套步进云台 `TB6612` 工程被整体迁移
2. 新目录为：

```text
F407 main control panel TB6612 Stepper motor pan-tilt version
```

3. 新目录下的 `code/5.15_F407_TB6612` 与旧路径中被 Git 跟踪的代码树一一对应

这说明本次改动主轴是“目录重构”，而不是“工程源码被大面积重写”。

## 3. 代码树核对结果

对比结果显示：

- 旧目录中已跟踪的代码树文件数：`126`
- 新目录中 `code/5.15_F407_TB6612` 的对应代码树文件数：`126`
- 二者内容映射完全对应

这意味着从代码树角度看，本次可以理解为“整套工程搬家”，不是“代码树内容在搬迁过程中发生了额外漂移”。

## 4. 额外新增的两份文件

在这次目录重构基础上，新目录还新增了两份原理图资料文件：

- `PCB schematic diagram/Stepper motor pan-tilt version.enet`
- `PCB schematic diagram/Stepper motor pan-tilt version.epro2`

这两份文件不属于旧路径中原先被 Git 跟踪的代码树，是这次重构之外的明确新增内容。

## 5. Git 处理策略

本轮建议按一次清晰的目录重构提交处理：

- 保留旧路径删除
- 加入新路径
- 让 Git 在提交时自行识别重命名/相似内容
- 同时纳入新增的两份原理图资料

这样可以把这次变化表达成：

- 一次结构调整
- 加两份新资料

而不是让仓库历史看起来像“整套工程删掉又重新写了一遍”。

## 6. 当前意义

这次整理之后，步进云台版本的 `TB6612` 工程将从原先的从属子目录，变成工作区根目录下的一级工程目录。

这对后续管理的好处是：

- 路径语义更清晰
- 与其他一级模块更对齐
- 后续继续为该分支单独开发、归档和版本管理会更直观
