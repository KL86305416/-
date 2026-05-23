# OpenMV 单文件调试脚本

这些脚本用于 OpenMV IDE 逐个功能测试。每个 `.py` 都是独立文件，不依赖项目根目录里的模块。

推荐顺序：

1. `00_camera_preview.py`：确认摄像头能出图。
2. `01_red_blob.py`：确认红色色块识别和串口输出。
3. `02_multi_color_blob.py`：确认红/绿/蓝阈值。
4. `03_line_follow.py`：确认黑线巡线输出。
5. `04_shape_detect.py`：确认圆和矩形识别。
6. `05_apriltag_detect.py`：确认 AprilTag 识别。
7. `06_qrcode_detect.py`：确认二维码识别。
8. `07_uart_protocol_test.py`：确认 `$MC` / `$MV` 串口协议。

使用方式：

- 在 OpenMV IDE 中打开其中一个脚本。
- 点击运行。
- 看帧缓冲区、串口终端和主控串口接收。
- 单文件跑通后，再回到项目根目录的 `main.py` 做整合。

注意：

- `UART(3)` 默认对应 OpenMV 常用 P4/P5。
- `x,y` 输出统一表示目标中心点。
- 正式比赛前必须在现场灯光下重新调 `THRESHOLD`。
