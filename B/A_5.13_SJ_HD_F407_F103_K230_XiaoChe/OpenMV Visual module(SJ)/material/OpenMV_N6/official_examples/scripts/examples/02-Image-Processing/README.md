# 02-Image-Processing — 图像处理例程

图像处理相关例程，涵盖绘图、滤镜、颜色追踪和帧差法运动检测。

## 00-Drawing（绘图）

在图像上绘制各种图形和文字。

| 例程 | 说明 |
|---|---|
| `line_drawing.py` | 直线绘制 |
| `rectangle_drawing.py` | 矩形绘制 |
| `circle_drawing.py` | 圆形绘制 |
| `ellipse_drawing.py` | 椭圆绘制 |
| `arrow_drawing.py` | 箭头绘制 |
| `cross_drawing.py` | 十字线绘制 |
| `text_drawing.py` | 文字叠加 |
| `image_drawing.py` | 通用图像绘制 |
| `image_drawing_advanced.py` | 高级绘制功能 |
| `image_drawing_alpha_blending_test.py` | Alpha 混合测试 |
| `image_drawing_alpha_blending_with_color_table_test.py` | Alpha 混合 + 颜色表 |
| `image_drawing_alpha_table_test.py` | Alpha 表测试 |
| `image_drawing_alpha_table_with_color_table_test.py` | Alpha 表 + 颜色表 |
| `image_drawing_with_custom_palette.py` | 自定义调色板绘制 |
| `image_drawing_scale_up_test.py` | 图像放大绘制 |
| `image_drawing_scale_down_test.py` | 图像缩小绘制 |
| `keypoints_drawing.py` | 关键点绘制 |
| `flood_fill.py` | 泛洪填充 |
| `copy2fb.py` | 复制图像到帧缓冲 |

## 01-Image-Filters（图像滤镜）

图像增强、变换和滤波处理。

**模糊/平滑类**：
- `blur_filter.py` — 均值模糊
- `mean_filter.py` — 均值滤波
- `median_filter.py` — 中值滤波（去椒盐噪声）
- `mode_filter.py` — 众数滤波
- `midpoint_filter.py` — 中点滤波
- `color_bilateral_filter.py` — 彩色双边滤波（保边去噪）
- `grayscale_bilateral_filter.py` — 灰度双边滤波

**锐化类**：
- `sharpen_filter.py` — 锐化
- `unsharp_filter.py` — 反锐化掩膜

**边缘检测**：
- `edge_filter.py` — 边缘检测
- `kernel_filters.py` — 自定义卷积核滤波

**二值化/阈值**：
- `color_binary_filter.py` — 彩色二值化
- `grayscale_binary_filter.py` — 灰度二值化
- `mean_adaptive_threshold_filter.py` — 均值自适应阈值
- `median_adaptive_threshold_filter.py` — 中值自适应阈值
- `midpoint_adaptive_threshold_filter.py` — 中点自适应阈值
- `mode_adaptive_threshold_filter.py` — 众数自适应阈值

**直方图/亮度**：
- `histogram_equalization.py` — 直方图均衡化
- `adaptive_histogram_equalization.py` — 自适应直方图均衡化（CLAHE）
- `gamma_correction.py` — Gamma 校正
- `negative.py` — 负片效果

**色彩/光线**：
- `color_correction.py` — 色彩校正
- `color_light_removal.py` — 彩色光照去除
- `grayscale_light_removal.py` — 灰度光照去除

**形态学**：
- `erode_and_dilate.py` — 腐蚀与膨胀

**几何变换**：
- `lens_correction.py` — 镜头畸变校正
- `rotation_correction.py` — 旋转校正
- `perspective_correction.py` — 透视校正
- `perspective_and_rotation_correction.py` — 透视 + 旋转校正
- `linear_polar.py` — 线性极坐标变换
- `log_polar.py` — 对数极坐标变换
- `vflip_hmirror_transpose.py` — 垂直翻转/水平镜像/转置

**其他**：
- `ulab.py` — 使用 ulab 库进行数值计算处理图像

## 02-Color-Tracking（颜色追踪）

基于颜色阈值的 blobs 检测与追踪。

| 例程 | 说明 |
|---|---|
| `single_color_rgb565_blob_tracking.py` | 单色 RGB565 色块追踪 |
| `single_color_grayscale_blob_tracking.py` | 单色灰度色块追踪 |
| `multi_color_blob_tracking.py` | 多色色块同时追踪 |
| `single_color_code_tracking.py` | 单色二维码（Color Code）追踪 |
| `multi_color_code_tracking.py` | 多色二维码追踪 |
| `automatic_rgb565_color_tracking.py` | 自动阈值 RGB565 颜色追踪 |
| `automatic_grayscale_color_tracking.py` | 自动阈值灰度颜色追踪 |
| `black_grayscale_line_following.py` | 黑线巡线（灰度） |
| `ir_beacon_grayscale_tracking.py` | 红外信标灰度追踪 |
| `ir_beacon_rgb565_tracking.py` | 红外信标 RGB565 追踪 |
| `image_histogram_info.py` | 图像直方图信息获取 |
| `image_statistics_info.py` | 图像统计信息（均值、标准差等） |

## 03-Frame-Differencing（帧差法）

通过前后帧比较检测运动/变化。

| 例程 | 说明 |
|---|---|
| `in_memory_basic_frame_differencing.py` | 内存帧差法基础：在 RAM 中比较前后帧 |
| `in_memory_advanced_frame_differencing.py` | 内存帧差法高级：多帧累积差分 |
| `on_disk_basic_frame_differencing.py` | 磁盘帧差法基础：使用 SD 卡存储参考帧 |
| `on_disk_advanced_frame_differencing.py` | 磁盘帧差法高级：SD 卡 + 多帧差分 |
| `in_memory_structural_similarity.py` | SSIM 结构相似度（内存） |
| `on_disk_structural_similarity.py` | SSIM 结构相似度（磁盘） |

---

**N6 注意**：所有图像处理例程均可在 N6 上运行。N6 的 NPU 不直接加速这些传统 CV 操作，但 STM32N6 的 Cortex-M55 @800MHz 性能足以流畅处理。
