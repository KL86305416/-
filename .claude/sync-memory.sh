#!/bin/bash
# 自动同步 CLAUDE.md —— 供 Claude Code Stop 钩子调用
cd "$(dirname "$0")/.."

git add CLAUDE.md A/CLAUDE.md

# 检查暂存区是否有变化，有则提交推送
if git diff --cached --quiet; then
    exit 0
fi

git commit -m "[auto] 同步 AI 记忆" || exit 0

# 网络对 GitHub 握手不稳，最多重试 3 次
for i in 1 2 3; do
    if git push; then
        exit 0
    fi
    sleep 1
done
exit 1
