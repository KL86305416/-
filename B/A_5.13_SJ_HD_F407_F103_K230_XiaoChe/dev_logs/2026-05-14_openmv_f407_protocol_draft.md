# OpenMV 与 F407 通讯协议草案

- 日期：2026-05-14
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`v0.1 draft`
- 目标：为 `OpenMV <-> F407` 设计一套简单、稳定、可快速落代码的串口协议。

## 1. 设计前提

当前已确认：

- 视觉主路线是 `OpenMV`
- `F407` 通过 `UART4` 与视觉侧通信
- `F407` 同时给视觉侧提供 `12V` 供电

因此本草案默认：

- `F407` 是主控
- `OpenMV` 是视觉协处理模块
- 通讯内容以“高层识别结果”为主，不传原始图像

## 2. 物理层约定

| 项目 | 约定 |
| --- | --- |
| 接口 | `F407 UART4 <-> OpenMV UART` |
| 波特率 | `115200` |
| 数据格式 | `8N1` |
| 字节序 | 小端 |
| 帧风格 | 二进制定长头 + 变长 payload + CRC16 |

## 3. 统一串口帧封装

建议 `OpenMV` 与 `F103` 的 UART 都复用同一套基础帧，减少 `F407` 侧解析器数量。

### 3.1 帧格式

| 字段 | 字节数 | 说明 |
| --- | --- | --- |
| `SOF1` | 1 | 固定 `0xAA` |
| `SOF2` | 1 | 固定 `0x55` |
| `VER` | 1 | 协议版本，当前 `0x01` |
| `SRC` | 1 | 源设备，`0x10 = OpenMV`，`0x01 = F407` |
| `TYPE` | 1 | 消息类型 |
| `SEQ` | 1 | 帧序号，循环递增 |
| `LEN` | 2 | payload 长度，`uint16` |
| `PAYLOAD` | `LEN` | 负载 |
| `CRC16` | 2 | 对 `VER..PAYLOAD` 做 `CRC16-CCITT-FALSE` |

### 3.2 CRC 约定

| 项目 | 值 |
| --- | --- |
| 多项式 | `0x1021` |
| 初值 | `0xFFFF` |
| 结果异或 | `0x0000` |
| 输入输出反转 | 否 |

## 4. 通讯角色

### 4.1 F407 -> OpenMV

`F407` 负责：

- 上电后下发工作模式
- 修改视觉参数
- 启停识别任务
- 监控心跳与状态

### 4.2 OpenMV -> F407

`OpenMV` 负责：

- 上报当前状态
- 上报识别结果
- 上报调试文本或错误码

## 5. 模式定义

| 模式值 | 名称 | 说明 |
| --- | --- | --- |
| `0x00` | `IDLE` | 空闲，不输出识别结果 |
| `0x01` | `COLOR_BLOB` | 颜色块识别 |
| `0x02` | `APRILTAG` | AprilTag |
| `0x03` | `SIGN_CLASSIFY` | 标志/牌面分类 |
| `0x04` | `LINE_AUX` | 视觉辅助线特征 |
| `0x05` | `CUSTOM` | 自定义模式 |

说明：

- 后续如果项目只用其中一种，可以保留模式值不变，只删实现。

## 6. 消息类型

### 6.1 F407 -> OpenMV

| `TYPE` | 名称 | 负载 |
| --- | --- | --- |
| `0x01` | `PING` | 空 |
| `0x02` | `MODE_SET` | `mode:u8` |
| `0x03` | `RUN_CTRL` | `0=stop, 1=start` |
| `0x04` | `PARAM_SET` | 参数块 |
| `0x05` | `REQ_STATUS` | 空 |
| `0x06` | `REQ_SNAPSHOT` | 空 |
| `0x7F` | `ACK` | `acked_type:u8, result:u8` |

### 6.2 OpenMV -> F407

| `TYPE` | 名称 | 负载 |
| --- | --- | --- |
| `0x10` | `HELLO` | 版本和能力 |
| `0x11` | `STATUS` | 模块状态 |
| `0x12` | `VISION_RESULT` | 主识别结果 |
| `0x13` | `DEBUG_TEXT` | 调试文本，可选 |
| `0x14` | `ERROR_REPORT` | 错误码 |
| `0x7F` | `ACK` | `acked_type:u8, result:u8` |

## 7. 关键负载定义

### 7.1 `HELLO`

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `dev_id` | `u8` | 固定 `0x10` |
| `fw_major` | `u8` | 主版本 |
| `fw_minor` | `u8` | 次版本 |
| `capability_mask` | `u16` | 能力位图 |

### 7.2 `STATUS`

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `state` | `u8` | `0=idle,1=running,2=error` |
| `mode` | `u8` | 当前模式 |
| `fps_x10` | `u16` | 帧率乘 10 |
| `latency_ms` | `u16` | 单帧平均时延 |
| `err_flags` | `u16` | 错误位图 |
| `uptime_s` | `u16` | 运行秒数低 16 位 |

### 7.3 `VISION_RESULT`

建议负载：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `mode` | `u8` | 当前识别模式 |
| `target_count` | `u8` | 当前检测到的目标数 |
| `label` | `u8` | 主目标类别 |
| `confidence` | `u8` | 0~100 |
| `cx` | `s16` | 主目标中心 x |
| `cy` | `s16` | 主目标中心 y |
| `w` | `u16` | 宽度 |
| `h` | `u16` | 高度 |
| `angle_x10` | `s16` | 角度乘 10 |
| `distance_x10` | `u16` | 距离乘 10，可无效填 0 |
| `flags` | `u16` | 状态位 |
| `frame_id` | `u16` | 帧序号 |

### 7.4 `PARAM_SET`

建议采用固定参数块：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `param_id` | `u8` | 参数编号 |
| `value0` | `u16` | 参数值 0 |
| `value1` | `u16` | 参数值 1 |
| `value2` | `u16` | 参数值 2 |

常见用途：

- ROI 设置
- 阈值设置
- 最小目标面积
- 置信度门限

## 8. 标签定义建议

项目标签建议先留统一表，不要在代码里散落 magic number。

| 值 | 含义 |
| --- | --- |
| `0x00` | `NONE` |
| `0x01` | `LEFT_MARK` |
| `0x02` | `RIGHT_MARK` |
| `0x03` | `STOP_MARK` |
| `0x04` | `TURN_BACK` |
| `0x10` | `RED_TARGET` |
| `0x11` | `GREEN_TARGET` |
| `0x12` | `BLUE_TARGET` |
| `0x20` | `APRILTAG_GENERIC` |
| `0x30` | `CUSTOM_0` |

说明：

- 这张表只是草案，真正落代码前可以按比赛任务再细化。

## 9. 时序建议

### 9.1 上电握手

1. `F407` 上电初始化 `UART4`
2. `OpenMV` 发 `HELLO`
3. `F407` 回 `ACK`
4. `F407` 发 `MODE_SET`
5. `F407` 发 `RUN_CTRL(start)`

### 9.2 运行中

- `OpenMV` 每处理完一帧，发送一帧 `VISION_RESULT`
- `OpenMV` 每 `200 ms` 至少发一次 `STATUS`
- `F407` 若 `500 ms` 没收到 `STATUS` 或 `VISION_RESULT`，判定视觉离线

### 9.3 调试期

- `DEBUG_TEXT` 只在开发期打开
- 比赛或正式运行时关闭，避免占用带宽

## 10. F407 侧建议处理逻辑

- 只接受 `CRC` 正确的帧
- 丢弃 `SEQ` 回退的旧帧
- `VISION_RESULT` 超时后清空视觉有效标志
- 将视觉结果缓存为结构体，不在中断里直接做业务决策

## 11. OpenMV 侧建议处理逻辑

- 用一个统一的 `send_frame(type, payload)` 封装发送
- `VISION_RESULT` 只发“主目标”
- 如果后续确实要多目标，再新增 `MULTI_TARGET`，不要提前做复杂化

## 12. 当前推荐落地顺序

1. 先实现 `HELLO / STATUS / MODE_SET / RUN_CTRL`
2. 再实现 `VISION_RESULT`
3. 最后再补 `PARAM_SET` 和 `DEBUG_TEXT`

这样能最快把 `OpenMV <-> F407` 联调跑通。

