# 2026-05-23 移除 OLED 按键提示显示

## 目标

- 去掉 OLED 界面中显示按键功能和按键状态的文字。
- 保留按键实际控制逻辑不变。

## 改动

- 从主菜单移除按键提示：
  - `PC3/2 Up/Dn PC1OK`
  - `PC0Back B3210:....`
- 删除主菜单按键原始状态实时刷新逻辑：
  - 移除 `APP_MENU_STATUS_UPDATE_MS`。
  - 移除 `App_ButtonRawPressedMask()`。
- 从舵机调试页移除按键提示：
  - `S1 PC3+ PC2-`
  - `S2 PC1+ PC0-`
  - `No Back Key`

## 保留项

- PC3/PC2/PC1/PC0 的按键扫描和实际功能保持不变。
- 主菜单仍显示 `Line Follow` 和 `Servo Debug`。
- 舵机调试页仍显示舵机通道和角度。

## 验证

- 已执行 `cmake --build build\Debug`。
- 编译通过。
- 已搜索确认 `B3210`、按键提示文字和按键原始状态显示函数无残留。

## 注意

- 当前工程目录不是 Git 仓库，无法执行提交。
