# OpenMV 校赛视觉项目

这是校级联赛用的 OpenMV 视觉工程。比赛主入口是 `main.py`，备用救援入口是 `main_safe.py`。当前工程按模块拆分：摄像头、识别、滤波、串口协议和调试绘制分别放在独立 `.py` 文件中，便于并行维护。

## 当前能力

| 模式 | 命令值 | 说明 |
|---|---|---|
| 空闲 | `idle` | 不执行目标识别，仍输出无目标结果 |
| 红色色块 | `red` | LAB 阈值色块识别 |
| 绿色色块 | `green` | LAB 阈值色块识别 |
| 蓝色色块 | `blue` | LAB 阈值色块识别 |
| 巡线 | `line` | 黑线区域识别，`angle` 表示相对偏移 |
| 形状 | `shape` | 圆形、矩形检测 |
| AprilTag | `apriltag` | AprilTag 检测，`tag`、`april` 为别名 |
| 二维码 | `qrcode` | QRCode 检测，`qr`、`code` 为别名 |
| 自动 | `auto` | 按 `qrcode`、`apriltag`、`shape`、`line`、`red`、`green`、`blue` 顺序尝试 |

默认模式来自 `config.py` 的 `DEFAULT_MODE`，当前为 `auto`。

## 文件结构

```text
.
├── main.py              # 比赛主入口，负责模式切换、识别调度和结果发送
├── main_safe.py         # 最小救援入口，只做红色色块识别，输出 ok,x,y
├── config.py            # 阈值、ROI、模式表、摄像头和串口参数
├── camera.py            # 摄像头初始化与取帧
├── comm.py              # UART 初始化、命令读取、结果帧发送
├── protocol.py          # $...*XX 帧格式和 XOR 校验
├── result.py            # 统一结果字段
├── blob.py              # 色块识别
├── line.py              # 巡线识别
├── shape.py             # 圆形、矩形识别
├── tag_code.py          # AprilTag 与二维码识别
├── filter.py            # 结果平滑与短时丢失保持
├── debug.py             # 调试绘制、日志、截图保存
├── guard.py             # 安全调用辅助
├── standalone/          # 单文件调试脚本，不依赖项目模块
└── docs/
    ├── usage.md         # OpenMV IDE 部署与调参流程
    ├── protocol.md      # 串口协议细节
    └── checklist.md     # 赛前检查清单
```

## OpenMV 部署

正常比赛把下列文件复制到 OpenMV 盘符根目录：

```text
main.py
config.py
camera.py
comm.py
protocol.py
result.py
blob.py
line.py
shape.py
tag_code.py
filter.py
debug.py
guard.py
```

`main.py` 会优先调用这些模块。缺少部分识别模块时，主入口仍会尝试用内置保底逻辑运行，但对应高级能力会降级。`comm.py` 依赖 `protocol.py`，这两个文件必须一起部署，否则串口协议帧发送会失败。

`main_safe.py` 是应急脚本，不参与 `main.py` 的模块化流程。建议同时保存到电脑和 OpenMV 盘符；需要救援时可以在 OpenMV IDE 中直接运行它，或把它改名为 `main.py` 后上电自启动。

## 运行与调参

1. 先运行 `main_safe.py`，确认摄像头、红色阈值、UART 线路和供电正常。
2. 再运行 `main.py`，用串口助手或主控切换 `red`、`green`、`blue`、`line`、`shape`、`apriltag`、`qrcode`、`auto`。
3. 在赛场灯光下调整 `config.py` 的 `THRESHOLDS`、`ROIS`、`MODES[*].min_area`。
4. 调参时可打开 `DEBUG_DRAW`，正式比赛前按帧率情况关闭。
5. 每次参数确认后断电重启 OpenMV，连续联调至少 3 分钟。

详细步骤见 [docs/usage.md](docs/usage.md)，赛前逐项核对见 [docs/checklist.md](docs/checklist.md)。

## 单文件逐项调试

如果你只想在 OpenMV IDE 里一个文件一个文件跑，不想处理模块依赖，使用 [standalone/](standalone/) 目录。

推荐顺序：

```text
standalone/00_camera_preview.py
standalone/01_red_blob.py
standalone/02_multi_color_blob.py
standalone/03_line_follow.py
standalone/04_shape_detect.py
standalone/05_apriltag_detect.py
standalone/06_qrcode_detect.py
standalone/07_uart_protocol_test.py
```

这些脚本都是自包含文件，直接打开运行即可。单文件跑通后，再回到 `main.py` 做整合。

## 串口协议

默认串口参数：

| 项目 | 默认值 |
|---|---:|
| UART | 3 |
| 波特率 | 115200 |
| 输出间隔 | 50 ms |

调试命令支持纯文本：

```text
mode:red
mode=green
m:line
m=auto
idle
```

正式联调推荐使用带 XOR 校验的命令帧：

```text
$MC,MODE,red*7E
$MC,STOP*3A
```

OpenMV 结果帧：

```text
$MV,RESULT,<mode>,<ok>,<id>,<x>,<y>,<w>,<h>,<score>,<area>,<angle>,<payload>,<fps>,<miss>,<error>*XX
```

字段说明和校验算法见 [docs/protocol.md](docs/protocol.md)。

## 应急判断

| 现象 | 优先检查 |
|---|---|
| 上电不运行 | OpenMV 根目录是否存在 `main.py`，是否有旧文件同名覆盖 |
| 导入错误 | 是否漏传 `protocol.py`、`comm.py` 或其他业务模块 |
| 无图像 | USB 连接、摄像头排线、镜头、IDE 是否占用设备 |
| 无串口数据 | UART 端口、波特率、TX/RX 交叉、共地 |
| 色块乱跳 | LAB 阈值、赛场光照、自动增益、自动白平衡 |
| 高级模式无结果 | 先切回 `red` 或 `line`，确认基础链路后再查对应模块 |
| 比赛前突然异常 | 直接运行 `main_safe.py`，先保住红色色块和串口链路 |
