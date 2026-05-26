# OpenMV N6 官方屏幕实时取流显示记录
- 日期：`2026-05-22`
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`implemented`
- 范围：`OpenMV Visual module(SJ)\code / backups / dev_logs / chat_data`

## 1. 本轮目标
在 `OpenMV Visual module(SJ)\code` 下落一份可直接运行的 OpenMV 脚本，实现：

1. 从 `OpenMV N6` 摄像头实时抓帧
2. 将画面显示到官方提供的屏幕背板上
3. 后续再补上左上角 FPS 叠加

## 2. 本轮交付文件
新增：
- `OpenMV Visual module(SJ)\code\main.py`
- `OpenMV Visual module(SJ)\code\README.md`

## 3. 脚本能力
### 3.1 屏幕兼容方式
脚本顶部通过 `DISPLAY_KIND` 选择官方屏幕类型，当前支持：
- `lcd_1p8`
- `touch_lcd_2p3`

默认值当前设为：
- `touch_lcd_2p3`

### 3.2 运行流程
脚本使用：
- `csi.CSI()` 初始化摄像头
- `display.SPIDisplay()` 初始化官方 SPI 屏
- `cam.snapshot()` 实时抓帧
- `lcd.write()` 将图像写到屏幕

图像显示策略为：
- `image.CENTER | image.SCALE_ASPECT_KEEP`

也就是：
- 画面居中
- 保持宽高比缩放
- 不直接强行拉伸变形

### 3.3 FPS 叠加
在后续小幅更新中，已把左上角 FPS 叠加补进当前脚本：
- `frame.draw_string(2, 2, "FPS: %.1f" % clock.fps(), ...)`

同时保留：
- REPL 串口输出 FPS

## 4. 当前使用方式
用户只需：
1. 打开 `main.py`
2. 根据实际官方屏幕修改 `DISPLAY_KIND`
3. 上传到 `OpenMV N6`
4. 重启即可运行

## 5. 校验结果
已完成本地 Python 语法检查：
- 工具：`python -m py_compile`
- 文件：`OpenMV Visual module(SJ)\code\main.py`
- 结果：通过

说明脚本在语法层面没有明显错误。

## 6. 说明
本轮没有在本地对真实 `OpenMV N6 + 官方屏` 进行实机运行验证，因此当前状态是：
- 语法已验证
- API 选择按官方资料路径实现
- 实际屏幕型号与固件 API 细节仍以板上联调为准

## 7. 备份与归档
已新增：
- `backups\2026-05-22_openmv_n6_display_liveview\README.md`
- `backups\2026-05-22_openmv_n6_display_liveview\main.py.snapshot`
- `backups\2026-05-22_openmv_n6_display_liveview\README.md.snapshot`
- `chat_data\2026-05-22_phase_24_openmv_n6_display_liveview.md`

本轮开始，OpenMV N6 这条线也继续按与 F407 主线一致的方式做：
- 代码快照
- 技术日志
- 阶段摘要
