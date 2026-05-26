# 01-Camera — 摄像头例程

摄像头相关例程，涵盖拍照、录像、光流、事件相机、热成像、ToF 测距、传感器参数控制等。

## 00-Snapshot（快照 / 拍照）

| 例程 | 说明 |
|---|---|
| `snapshot.py` | 基础拍照：按用户按键后将当前帧保存为 BMP 到 SD 卡 |
| `snapshot_on_face_detection.py` | 人脸触发拍照：检测到人脸时自动拍照并保存 |
| `snapshot_on_movement.py` | 运动触发拍照：检测到画面移动时自动拍照 |
| `emboss_snapshot.py` | 浮雕效果拍照：对图像做浮雕滤镜后保存 |
| `time_lapse_photos.py` | 延时摄影：按设定时间间隔自动拍照 |

## 01-Video-Recording（录像）

| 例程 | 说明 |
|---|---|
| `mjpeg.py` | MJPEG 录像：将摄像头帧录制为 MJPEG 文件保存到 SD 卡 |
| `mjpeg_on_face_detection.py` | 人脸触发录像：检测到人脸时开始录制 |
| `mjpeg_on_movement.py` | 运动触发录像：检测到运动时开始录制 |
| `gif.py` | GIF 录制：将帧录制为 GIF 动图 |
| `gif_on_face_detection.py` | 人脸触发 GIF：检测到人脸时开始录制 GIF |
| `gif_on_movement.py` | 运动触发 GIF：检测到运动时开始录制 GIF |
| `imageio_memory.py` | 内存读写图像：在内存中读写 ImageIO 格式 |
| `imageio_read.py` | 读取图像文件：从 SD 卡读取 ImageIO 格式文件 |
| `imageio_write.py` | 写入图像文件：将帧写入 SD 卡为 ImageIO 格式 |

## 02-Optical-Flow（光流 / 位移检测）

基于相位相关算法的图像位移测量，适用于机器人定位、运动检测。

| 例程 | 说明 |
|---|---|
| `absolute-translation.py` | 绝对位移检测：相比参考帧的 X/Y 平移量 |
| `absolute-rotation-scale.py` | 绝对旋转/缩放检测：相比参考帧的旋转角度和缩放比例 |
| `differential-translation.py` | 差分位移检测：相邻帧之间的平移量 |
| `differential-rotation-scale.py` | 差分旋转/缩放检测：相邻帧之间的旋转和缩放 |
| `image-patches-absolute-translation.py` | 图像块绝对位移：分区域检测平移，适合局部运动分析 |
| `image-patches-absolute-rotation-scale.py` | 图像块绝对旋转/缩放 |
| `image-patches-differential-translation.py` | 图像块差分平移 |
| `image-patches-differential-rotation-scale.py` | 图像块差分旋转/缩放 |

## 03-Event-Cameras（事件相机）

适用于 Frogeye2020 和 Genx320 等事件相机传感器。

### 01-Frogeye2020

| 例程 | 说明 |
|---|---|
| `frogeye2020.py` | Frogeye2020 基础显示：320x240 事件相机，每像素 2bit 表示无运动/方向A/方向B |
| `frogeye2020_with_tracking.py` | Frogeye2020 + 追踪：事件相机配合颜色追踪 |

### 02-Genx320

Genx320 事件相机（共 18 个例程），分为两大模式：

**事件模式**（原始事件输出）：
- `genx320_event_mode.py` — 基础事件模式
- `genx320_event_mode_biases.py` — 偏置参数调节
- `genx320_event_mode_calibration.py` — 传感器校准
- `genx320_event_mode_deep_buffer.py` — 深度缓冲模式
- `genx320_event_mode_deep_buffer_filtering.py` — 深度缓冲 + 滤波
- `genx320_event_mode_filtering.py` — 事件滤波
- `genx320_event_mode_high_speed.py` — 高速事件模式
- `genx320_event_mode_high_speed_filtering.py` — 高速 + 滤波
- `genx320_event_mode_long_exposure.py` — 长曝光事件模式
- `genx320_event_mode_long_exposure_filtering.py` — 长曝光 + 滤波
- `genx320_event_mode_set_stc.py` — 设置 STC 参数

**直方图模式**（帧化显示）：
- `genx320_histogram_color_mode.py` — 彩色直方图显示
- `genx320_histogram_color_mode_with_tracking.py` — 彩色直方图 + 追踪
- `genx320_histogram_grayscale_mode.py` — 灰度直方图显示
- `genx320_histogram_grayscale_mode_set_afk.py` — 设置 AFK 参数
- `genx320_histogram_grayscale_mode_with_tracking.py` — 灰度直方图 + 追踪
- `genx320_histogram_mode_calibration.py` — 直方图模式校准
- `genx320_histogram_mode_grayscale_led_tracking.py` — 灰度直方图 LED 追踪

## 04-Global-Shutter（全局快门）

适用于 MT9V0X2/MT9V0X4 等全局快门传感器。

| 例程 | 说明 |
|---|---|
| `high_fps.py` | 高帧率模式：配置全局快门传感器高速采集 |
| `triggered_mode.py` | 触发模式：外部信号触发拍照 |

## 05-Thermal-Cameras（热成像相机）

### 01-FLIR-Lepton

FLIR Lepton 热成像传感器例程（共 13 个）：

| 例程 | 说明 |
|---|---|
| `lepton_get_object_temp.py` | 获取物体温度：读取 Lepton 测量的物体温度 |
| `lepton_get_object_temp_color.py` | 温度 + 伪彩色显示 |
| `lepton_get_object_temp_lcd.py` | 温度 + LCD 屏显示 |
| `lepton_get_object_temp_color_lcd.py` | 温度 + 伪彩色 + LCD 显示 |
| `lepton_get_object_high_temp.py` | 获取高温目标 |
| `lepton_hotspot_grayscale_color_tracking.py` | 热点追踪（灰度/彩色） |
| `lepton_hotspot_grayscale_color_tracking_lcd.py` | 热点追踪 + LCD |
| `lepton_hotspot_rgb565_color_tracking.py` | 热点 RGB565 追踪 |
| `lepton_hotspot_rgb565_color_tracking_lcd.py` | 热点 RGB565 追踪 + LCD |
| `lepton_target_temp_hotspot_grayscale_color_tracking.py` | 目标温度热点追踪 |
| `lepton_target_temp_hotspot_rgb565_color_tracking.py` | 目标温度 RGB565 热点追踪 |
| `lepton_with_color_sensor_overlay.py` | 热成像 + 彩色摄像头叠加 |
| `lepton_with_color_sensor_overlay_with_transform.py` | 热成像 + 彩色叠加 + 透视变换（含 N6 适配） |

### 02-FLIR-Boson

FLIR Boson 高端热成像传感器例程。

## 06-Time-of-Flight（ToF 测距）

| 例程 | 说明 |
|---|---|
| `tof_camera.py` | ToF 基础测距：读取 ToF 传感器距离数据 |
| `tof_overlay.py` | ToF 叠加显示：距离数据叠加到彩色图像上 |

## 07-Sensor-Control（传感器参数控制）

控制摄像头传感器的曝光、增益、白平衡、镜像翻转等参数。

| 例程 | 说明 |
|---|---|
| `sensor_exposure_control.py` | 曝光控制：手动/自动调节曝光时间 |
| `sensor_auto_gain_control.py` | 自动增益控制：开关及调节自动增益 |
| `sensor_manual_gain_control.py` | 手动增益控制：固定增益值 |
| `sensor_manual_whitebal_control.py` | 手动白平衡：固定白平衡参数 |
| `sensor_horizontal_mirror.py` | 水平镜像翻转 |
| `sensor_vertical_flip.py` | 垂直翻转 |
| `autofocus.py` | 自动对焦控制（需支持 AF 的传感器） |
| `sensor_save_and_restore_settings.py` | 保存/恢复传感器设置 |

## 08-Readout-Control（读出控制）

控制传感器读出区域和帧率。

| 例程 | 说明 |
|---|---|
| `100_fps_ir_led_tracking.py` | 100 FPS IR LED 追踪：高帧率下进行红外 LED 追踪 |
| `apriltag_tracking.py` | AprilTag 追踪：高帧率下进行 AprilTag 检测与追踪 |

---

**N6 注意**：N6 使用 `csi.CSI()` API（而非旧版 `sensor` 模块），多数摄像头例程可直接运行，但某些特殊传感器（Lepton、事件相机等）需对应硬件支持。
