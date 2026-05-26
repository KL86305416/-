# 06-April-Tags — AprilTag 标记检测

AprilTag 是一种视觉基准标记系统，常用于机器人定位、相机标定和增强现实。

| 例程 | 说明 |
|---|---|
| `find_apriltags.py` | 基础 AprilTag 检测（支持 16H5、25H7、36H11 等家族） |
| `find_apriltags_3d.py` | 3D 位姿估计：检测 AprilTag 并解算其 6-DOF 位姿（x, y, z, roll, pitch, yaw） |
| `find_apriltags_3d_with_pose_correction.py` | 3D 位姿 + 位姿校正优化 |
| `find_apriltags_max_res.py` | 全分辨率 AprilTag 检测（检测更远/更小的标签） |
| `find_apriltags_low_fps.py` | 低频检测模式（节省算力，适合低频场景） |

---

**N6 注意**：N6 800MHz CPU 可流畅运行全分辨率 AprilTag 检测。3D 位姿估计适合用 N6 做视觉引导/定位。
