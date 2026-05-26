# Contributing Guide

## 1. 提交目标

这个仓库不是单一代码目录，而是“代码 + 原理图 + 日志 + 阶段归档 + 备份”的完整研发工作区。

因此提交时的目标不是只让代码能编译，而是让这次改动：

- 范围清晰
- 意图清晰
- 记录可追溯

## 2. 推荐分支命名

推荐格式：

```text
<type>/<topic>
```

例如：

- `feat/f407-tb6612-line-follow`
- `fix/f103-runtime-short-frame`
- `docs/workspace-archive-maintenance`
- `chore/repo-cleanup`

## 3. 提交信息格式

推荐采用：

```text
type(scope): short summary
```

例如：

- `feat(f407-tb6612): add motor self-test routine`
- `fix(f103-gray): correct runtime flag packing`
- `docs(workspace): update archive index`
- `chore(repo): tighten git ignore rules`

## 4. 推荐 type

- `feat`
  说明：新增功能
- `fix`
  说明：修复问题
- `refactor`
  说明：重构，不改变外部行为
- `docs`
  说明：文档、日志、归档说明更新
- `chore`
  说明：仓库维护、配置、工程整理
- `test`
  说明：测试代码、联调入口、自检逻辑
- `backup`
  说明：与备份结构或备份说明相关的提交

## 5. 推荐 scope

建议尽量使用和工作区一致的范围命名：

- `workspace`
- `repo`
- `f103-gray`
- `f407-drv8874`
- `f407-tb6612`
- `openmv`
- `k230`
- `docs`
- `backup`

## 6. 提交粒度建议

一条提交尽量只做一类事情。

推荐做法：

- 协议改动单独提交
- 电机层改动单独提交
- 灰度接收链修复单独提交
- 仓库整理、忽略规则、文档规范单独提交

不推荐把这些混进一条提交里：

- 功能开发
- 编译产物变化
- 日志补写
- 目录重排

## 7. 与日志和备份的配合规则

以下情况建议同步更新 `dev_logs / chat_data / backups`：

- 改协议
- 改硬件映射
- 改主控底层
- 修关键联调问题
- 做风险较高的批量调整

推荐顺序：

1. 先备份关键文件
2. 做代码改动
3. 做最小验证
4. 更新日志与阶段摘要
5. 再提交

## 8. 提交前自检

提交前至少确认：

- 没把明显编译垃圾带进仓库
- 改动范围和提交标题一致
- 如涉及硬件逻辑，日志已补
- 如涉及高风险文件，备份已补
- `git status` 里没有意外文件

## 9. 推荐初始命令

当前仓库已经配置了本地提交模板，可直接使用：

```powershell
git commit
```

或自行写明确消息：

```powershell
git commit -m "fix(f407-tb6612): repair motor direction mapping"
```
