# 03-Machine-Learning — 机器学习 / AI 例程

OpenMV 的 AI 推理例程，涵盖 TensorFlow Lite 模型、ST CubeAI 和 Haar 级联检测。

## 00-TensorFlow（TensorFlow Lite 推理）

基于 TFLite 模型的深度学习推理（共 15 个例程）。

**YOLO 目标检测**：
| 例程 | 说明 |
|---|---|
| `yolo_v2_detector.py` | YOLOv2 目标检测器 |
| `yolo_v5_detector.py` | YOLOv5 目标检测器 |
| `yolo_v8_detector.py` | YOLOv8 目标检测器（N6 板载 `/rom/yolov8n_192.tflite`） |
| `yolo_lc_person_detector.py` | YOLO 轻量行人检测 |

**人脸相关**：
| 例程 | 说明 |
|---|---|
| `blazeface_detector.py` | BlazeFace 人脸检测 |
| `face_landmarks_single_face.py` | 单人人脸关键点（468 点） |
| `face_landmarks_multi_face.py` | 多人人脸关键点 |

**手势/姿态**：
| 例程 | 说明 |
|---|---|
| `blazepalm_detection.py` | BlazePalm 手掌检测 |
| `hand_landmarks_single_hand.py` | 单手手部关键点（21 点） |
| `hand_landmarks_multi_hand.py` | 多手手部关键点 |
| `movenet_singlepose_detection.py` | MoveNet 单人姿态估计（17 个关键点） |

**通用推理**：
| 例程 | 说明 |
|---|---|
| `tf_image_classification.py` | 图像分类（如 MobileNet） |
| `tf_object_detection.py` | 通用目标检测（非 YOLO 格式） |
| `tf_regression.py` | 回归模型推理 |
| `micro_speech.py` | 语音指令识别（Micro Speech） |

## 01-ST-CubeAI（ST CubeAI 推理）

| 例程 | 说明 |
|---|---|
| `nn_stm32cubeai.py` | 使用 STM32CubeAI 运行时进行神经网络推理 |

## 02-Haar-Cascade（Haar 级联检测）

基于传统 Haar 特征的级联分类器。

| 例程 | 说明 |
|---|---|
| `face_detection.py` | 人脸检测（Haar Cascade） |
| `face_eye_detection.py` | 人脸 + 人眼检测 |
| `face_tracking.py` | 人脸追踪（检测 + PID 追踪控制） |
| `face_recognition.py` | 人脸识别（检测 + 特征比对） |
| `iris_detection.py` | 虹膜检测 |

---

**N6 注意**：
- N6 的 NPU（600 GOPS）可大幅加速 TFLite 模型推理。运行前需通过 IDE 的 `Tools → Machine Vision → Convert Model for NPU` 转换模型。
- YOLOv8n 人物检测模型已预置在 `/rom/yolov8n_192.tflite`，可直接运行。
- ST-CubeAI 例程仅适用于 STM32 系列板卡（含 N6）。
