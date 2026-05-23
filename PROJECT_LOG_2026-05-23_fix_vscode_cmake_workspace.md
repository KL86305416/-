# 项目日志：修复 VS Code CMake 工程识别

- 日期：2026-05-23
- 本地仓库：C:\Users\admini\Desktop\校级联赛电赛\304
- 目标问题：VS Code 提示仓库包含多个 CMake projects，导致无法正常选择/构建 STM32 工程。

## 本次处理

1. 在仓库根目录新增 `.vscode/settings.json`。
2. 将 `cmake.sourceDirectory` 指定为 `${workspaceFolder}/A/STM32/FindLine`，让 CMake Tools 固定识别 STM32 主工程。
3. 将根目录 CMake 命令设置为已验证可用的 `cmake`，并保留 Ninja 生成器设置。
4. 在仓库根目录新增 `.vscode/c_cpp_properties.json`，将 C/C++ IntelliSense 的 `compileCommands` 指向 STM32 主工程构建目录。

## 验证结果

- `cmake --preset Debug` 配置成功。
- `cmake --build --preset Debug` 构建成功。
- 已生成 `A/STM32/FindLine/build/Debug/FindLine.elf`。
- 构建结果显示 RAM 使用 3248 B / 32 KB，FLASH 使用 20436 B / 128 KB。

## 使用建议

- 重新加载 VS Code 窗口后，CMake Tools 应不再提示多个 CMake projects。
- 如果仍有旧缓存提示，可执行 CMake: Delete Cache and Reconfigure。
