# 05-Feature-Detection — 特征检测

图像特征与几何形状检测。

| 例程 | 说明 |
|---|---|
| `edges.py` | 边缘检测（Canny 算法） |
| `find_blobs.py` | 色块/斑点检测 |
| `find_circles.py` | 霍夫圆检测 |
| `find_lines.py` | 霍夫直线检测 |
| `find_rects.py` | 矩形检测 |
| `find_line_segments.py` | 线段检测（LSD 算法） |
| `find_corners.py` | 角点检测（Harris/Fast） |
| `lbp.py` | LBP（局部二值模式）特征提取 |
| `lbp_keypoints_matching.py` | LBP 特征 + 关键点匹配 |
| `template_matching.py` | 模板匹配（在画面中搜索模板图像） |
| `keypoints_save.py` | 保存关键点到文件 |
| `keypoints_load.py` | 从文件加载关键点 |

---

**N6 注意**：本目录所有例程均可在 N6 上运行。高分辨率下特征检测效果更好但速度会下降。
