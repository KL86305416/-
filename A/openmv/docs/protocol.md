# 串口协议

## 串口参数

| 项目 | 默认值 |
|---|---:|
| UART | 3 |
| 波特率 | 115200 |
| 字符超时 | 20 |
| 结果输出间隔 | 50 ms |

参数来自 `config.py`：`UART_PORT`、`UART_BAUDRATE`、`SEND_INTERVAL_MS`。

## 帧格式

协议帧格式：

```text
$<body>*<checksum>\n
```

校验规则：

1. `body` 不包含开头 `$`，也不包含 `*` 和后面的校验值。
2. `checksum` 是 `body` 每个字符 ASCII 码逐字节 XOR 后的两位大写十六进制。
3. 字段分隔符为英文逗号 `,`。

示例：

```text
body = MC,MODE,red
checksum = 7E
frame = $MC,MODE,red*7E
```

## 主控到 OpenMV

正式命令前缀为 `MC`。

| 命令 | 示例 | 说明 |
|---|---|---|
| 切换模式 | `$MC,MODE,red*7E` | 切到红色色块模式 |
| 停止识别 | `$MC,STOP*3A` | 切到 `idle` |
| 保持当前模式 | `$MC,PING*XX` | 当前 `main.py` 接收后保持模式 |
| 保持当前模式 | `$MC,START*XX` | 当前 `main.py` 接收后保持模式 |
| 保持当前模式 | `$MC,DEBUG,1*XX` | 当前 `main.py` 接收后保持模式 |
| 保持当前模式 | `$MC,SAVE*XX` | 当前 `main.py` 接收后保持模式 |
| 保持当前模式 | `$MC,RESET*XX` | 当前 `main.py` 接收后保持模式 |

当前 `main.py` 实际只用 `MODE` 改模式、`STOP` 进 `idle`；其他命令会被识别但不改变当前模式。需要完整状态机应答时，后续再接入 `state.py` 的 `VisionState.handle_command()`。

支持模式：

```text
idle
red
green
blue
line
shape
apriltag
qrcode
auto
```

支持别名：

| 别名 | 实际模式 |
|---|---|
| `tag` | `apriltag` |
| `april` | `apriltag` |
| `qr` | `qrcode` |
| `code` | `qrcode` |
| `color` | `red` |

## 调试纯文本命令

未使用协议帧时，`main.py` 也支持直接读取文本命令：

```text
mode:red
mode=green
m:line
m=auto
idle
```

未知模式会保持当前模式。

## OpenMV 到主控

结果帧前缀为 `MV`：

```text
$MV,RESULT,<mode>,<ok>,<id>,<x>,<y>,<w>,<h>,<score>,<area>,<angle>,<payload>,<fps>,<miss>,<error>*XX
```

字段：

| 字段 | 说明 |
|---|---|
| `mode` | 当前识别模式；`auto` 命中后仍输出 `auto` |
| `ok` | `1` 表示识别到目标，`0` 表示未识别到 |
| `id` | 模式 ID 或目标 ID |
| `x` / `y` | 目标中心点坐标 |
| `w` / `h` | 目标外接框宽高 |
| `score` | 模块评分，色块和巡线通常为面积或像素数 |
| `area` | 面积 |
| `angle` | 角度或偏移量；巡线保底逻辑为相对中心偏移百分比 |
| `payload` | 二维码内容、标签附加信息或空字符串 |
| `fps` | 当前帧率整数 |
| `miss` | 未命中计数或未命中标记 |
| `error` | 异常信息，正常为空 |

## 救援脚本输出

`main_safe.py` 不使用协议帧，只输出：

```text
1,x,y
0,0,0
```

它用于应急验证，不建议主控把它和正式 `$MV,RESULT,...*XX` 协议混在同一解析分支里。
