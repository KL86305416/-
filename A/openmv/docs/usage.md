# OpenMV IDE 部署与调参流程

## 部署文件

### 正常比赛入口

把这些文件复制到 OpenMV 盘符根目录：

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

OpenMV 上电只会自动运行根目录的 `main.py`。如果电脑项目里改了参数，必须重新把 `config.py` 同步到 OpenMV 盘符；只在电脑上保存没有用。

### 救援入口

`main_safe.py` 只依赖 OpenMV 自带的 `sensor`、`time`、`pyb.UART`，用于快速确认红色色块、摄像头和 UART 是否还活着。

使用方式二选一：

1. 在 OpenMV IDE 中打开 `main_safe.py`，点击运行。
2. 现场需要脱机自启动时，把 OpenMV 盘符里的原 `main.py` 备份，再把 `main_safe.py` 改名为 `main.py`。

救援输出固定为：

```text
1,x,y
0,0,0
```

## OpenMV IDE 操作

1. USB 连接 OpenMV。
2. 打开 OpenMV IDE 并连接开发板。
3. 打开项目中的 `main_safe.py`，先验证画面、红色目标和串口。
4. 打开 `main.py`，逐个模式验证。
5. 确认稳定后，把部署文件保存到 OpenMV 盘符根目录。
6. 断电重启，确认不依赖 IDE 也能自动输出数据。

## 推荐调参顺序

1. 固定赛场灯光、摄像头高度、角度和镜头焦距。
2. 运行 `main_safe.py`，用红色目标调通最小链路。
3. 在 OpenMV IDE 的 Threshold Editor 中采样目标颜色。
4. 将 LAB 阈值写入 `config.py` 的 `THRESHOLDS`。
5. 调整 `ROIS`，只保留任务真正需要看的区域。
6. 调整 `MODES` 中对应模式的 `min_area`，过滤噪点或保留远处目标。
7. 回到 `main.py`，依次验证 `red`、`green`、`blue`、`line`、`shape`、`apriltag`、`qrcode`、`auto`。
8. 用主控连续读取至少 3 分钟，确认无卡死、乱码和字段错位。

## 参数位置

| 参数 | 文件位置 | 调整方向 |
|---|---|---|
| `THRESHOLDS` | `config.py` | 漏检就放宽，误检就收紧 |
| `ROIS` | `config.py` | 只覆盖比赛任务区域，避开干扰物 |
| `MODES[*].min_area` | `config.py` | 噪点多就增大，远距离漏检就减小 |
| `CAMERA_EXPOSURE_US` | `config.py` | 画面过暗增大，拖影明显减小 |
| `CAMERA_GAIN_DB` | `config.py` | 低光可增大，噪声多就减小 |
| `CAMERA_WHITEBAL` | `config.py` | 色块任务建议固定为 `False` |
| `SEND_INTERVAL_MS` | `config.py` | 主控处理不过来就增大 |
| `DEBUG_DRAW` | `config.py` | 调参时打开，正式比赛前按性能关闭 |
| `DEBUG_SAVE_IMAGE` | `config.py` | 只在需要留样排障时打开 |

## 串口调试

调试阶段可直接发纯文本：

```text
mode:red
mode:line
mode:auto
m=blue
idle
```

正式联调使用协议帧：

```text
$MC,MODE,red*7E
$MC,MODE,line*03
$MC,MODE,auto*02
$MC,STOP*3A
```

如果没有输出，按顺序检查：

1. `config.py` 中 `UART_PORT` 是否匹配板子。
2. 波特率是否为 `115200`。
3. OpenMV TX 是否接主控 RX，OpenMV RX 是否接主控 TX。
4. OpenMV 与主控是否共地。
5. OpenMV IDE 的串口终端是否占用或干扰测试方式。

## 保存与复测

每次确认参数后都做这三件事：

1. 把最新 `config.py` 和相关模块同步到 OpenMV 盘符。
2. 断电重启 OpenMV，确认 `main.py` 自动启动。
3. 让主控读取至少 30 秒，检查模式切换、结果字段和帧率。
