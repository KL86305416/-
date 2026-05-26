# F103 运行态短帧落地记录

- 日期：2026-05-14
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`implemented in code`
- 范围：仅涉及 `F103 grayscale sensor(HD)` 工程

## 1. 本次变更目标

把 `F103` 当前串口输出从调试长帧：

```text
AA 55 VER SRC TYPE SEQ LEN_L LEN_H PAYLOAD... CRC16_L CRC16_H
```

切换为运行态短帧：

```text
AA 55 TURN SCENE FLAGS CRC8
```

目标是：

- 减少串口帧长度
- 降低 `F407` 解析负担
- 让 `F103` 尽量在本地完成灰度理解

## 2. 本次实际改动文件

工程路径：
`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe\F103 grayscale sensor(HD)\code\4.28_test`

修改：

- `Core/Inc/gray_protocol.h`
- `Core/Src/gray_protocol.c`
- `Core/Src/gray_app.c`

## 3. 当前实际发送帧格式

当前 `USART1` 发出的正式运行帧为：

```text
AA 55 TURN SCENE FLAGS CRC8
```

总长固定 `6` 字节。

字段定义：

| 字节 | 字段 | 类型 | 说明 |
| --- | --- | --- | --- |
| 0 | `SOF1` | `u8` | 固定 `0xAA` |
| 1 | `SOF2` | `u8` | 固定 `0x55` |
| 2 | `TURN` | `s8` | 负左正右，绝对值表示修正强度 |
| 3 | `SCENE` | `u8` | 当前主场景 |
| 4 | `FLAGS` | `u8` | 当前补充状态位 |
| 5 | `CRC8` | `u8` | 对 `TURN+SCENE+FLAGS` 校验 |

## 4. CRC8 规则

当前代码采用：

- 多项式：`0x07`
- 初值：`0x00`
- 不反转
- 不额外异或

也就是一版简洁的 `CRC-8`。

## 5. 当前 `SCENE` 定义

| 值 | 名称 | 含义 |
| --- | --- | --- |
| `0x00` | `GRAY_SCENE_NONE` | 保留/未使用 |
| `0x01` | `GRAY_SCENE_STRAIGHT` | 普通直行跟线 |
| `0x02` | `GRAY_SCENE_SOFT_LEFT` | 普通左转趋势 |
| `0x03` | `GRAY_SCENE_SOFT_RIGHT` | 普通右转趋势 |
| `0x04` | `GRAY_SCENE_HARD_LEFT` | 急左 / 左直角趋势 |
| `0x05` | `GRAY_SCENE_HARD_RIGHT` | 急右 / 右直角趋势 |
| `0x06` | `GRAY_SCENE_CROSS` | 十字区 |
| `0x07` | `GRAY_SCENE_LEFT_BRANCH` | 左支路/左分叉 |
| `0x08` | `GRAY_SCENE_RIGHT_BRANCH` | 右支路/右分叉 |
| `0x09` | `GRAY_SCENE_T_JUNCTION` | T 字或近似 T 字 |
| `0x0A` | `GRAY_SCENE_STOP_BAR` | 停车线 / 大面积黑区 |
| `0x0B` | `GRAY_SCENE_LINE_LOST` | 丢线 |
| `0x0C` | `GRAY_SCENE_AMBIGUOUS` | 复杂/不稳定 |

## 6. 当前 `FLAGS` 定义

| 位 | 名称 | 含义 |
| --- | --- | --- |
| bit0 | `VALID` | 当前输出有效 |
| bit1 | `LINE_FOUND` | 当前检测到主线 |
| bit2 | `LEFT_FEATURE` | 左侧存在明显结构特征 |
| bit3 | `RIGHT_FEATURE` | 右侧存在明显结构特征 |
| bit4 | `CENTER_ON_LINE` | 中轴附近仍压在线上 |
| bit5 | `WIDE_BLACK` | 出现大面积连续黑区 |
| bit6 | `UNSTABLE` | 当前判定不稳定或复杂 |
| bit7 | `SENSOR_FAULT` | 预留给传感器异常 |

## 7. 当前 `TURN` 生成规则

### 7.1 基础量

当前先由：

- `center_err * 2`

得到基础 `TURN`，再限制到：

- `-100 ~ +100`

其中：

- 负值：左修正
- 正值：右修正

### 7.2 场景修正

当前又做了场景级再修正：

- `HARD_LEFT`
  - 至少输出 `-60`
- `HARD_RIGHT`
  - 至少输出 `+60`
- `STOP_BAR`
  - 输出 `0`
- `LINE_LOST`
  - 使用“上一次有效转向方向”做恢复
  - 当前恢复量固定为 `±25`

## 8. 当前场景判定的一阶逻辑

当前不是最终版，而是第一轮可运行规则：

### 8.1 丢线

- `active_count == 0`
- 输出：
  - `SCENE_LINE_LOST`

### 8.2 停车线 / 超大黑区

- `sensor_bits == 0xFF`
- 或者：
  - 大面积黑区
  - 左右都很强
  - 中心也压线
  - 激活路数达到 7 及以上

### 8.3 十字

- 中心压线
- 左右同时有明显结构特征
- 激活路数达到 5 及以上

### 8.4 T 字

- 中心不压线
- 左右都出现明显结构特征
- 激活路数达到 4 及以上

### 8.5 左/右分支

- 中心仍压线
- 一侧出现明显结构特征
- 另一侧没有形成对称结构

### 8.6 急弯 / 直角

- `TURN` 幅度超过硬阈值
- 或一侧明显聚集且另一侧丢失

### 8.7 普通转向

- `TURN` 超过软阈值但未达到急弯条件

### 8.8 直行

- 不满足上面条件
- 且当前不处于不稳定态

## 9. 与之前长帧的关系

当前代码里：

- 长帧结构和状态缓存仍然保留
- 但运行发送路径已经切换到短帧

也就是说：

- 板上真正发出来的是短帧
- 不是之前那套 `LEN=8` 的调试长帧

## 10. 已完成校验

已完成：

- 语法编译检查

命令：

```powershell
gcc -fsyntax-only -DUSE_HAL_DRIVER -DSTM32F103xB -ICore\Inc -IDrivers\STM32F1xx_HAL_Driver\Inc -IDrivers\CMSIS\Device\ST\STM32F1xx\Include -IDrivers\CMSIS\Include Core\Src\main.c Core\Src\gray_app.c Core\Src\gray_protocol.c
```

结果：

- 通过
- 仅存在 CMSIS 在主机编译下常见的指针宽度 warning
- 本次新增业务代码无语法错误

## 11. 下一步建议

下一步最值得做的是上板抓短帧，并确认这四件事：

1. `TURN` 的符号方向是否符合“左负右正”
2. `TURN` 的幅度是否满足实际车体修正需要
3. `SCENE` 在十字、直角、普通弯、丢线场景下是否判得对
4. `FLAGS` 是否足够稳定，不会频繁误跳

只有这四项确认后，短帧才算真正进入可接主控的状态。
