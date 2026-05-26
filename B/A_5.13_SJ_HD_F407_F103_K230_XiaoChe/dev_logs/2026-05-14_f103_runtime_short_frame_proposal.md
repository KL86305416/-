# F103 运行态短帧协议提案

- 日期：2026-05-14
- 工作区：`D:\A_5.13_SJ_HD_F407_F103_K230_XiaoChe`
- 状态：`runtime short frame proposal`
- 目标：让 `F103` 尽量完成本地灰度理解，只向 `F407` 上传“该往哪边修、修多少、当前是什么路段场景”

## 1. 设计原则

本提案的核心不是“只压缩字节数”，而是同时做到：

- 主控少解析
- 主控少计算
- 状态表达足够覆盖常见赛道场景
- 帧长仍保持很短

因此不建议运行态只发：

- `sensor_bits`

因为那样虽然只有 1 字节数据，但 `F407` 仍然要自己做：

- 线型识别
- 交叉判断
- 直角弯判断
- 偏差映射

这不符合“尽量替主控减少算力”的目标。

## 2. 推荐运行态短帧

推荐采用：

```text
AA 55 TURN SCENE FLAGS CRC8
```

总长 `6` 字节。

字段说明：

| 字节序号 | 字段 | 类型 | 说明 |
| --- | --- | --- | --- |
| 0 | `SOF1` | `u8` | 固定 `0xAA` |
| 1 | `SOF2` | `u8` | 固定 `0x55` |
| 2 | `TURN` | `s8` | 转向建议，负左正右，绝对值表示修正强度 |
| 3 | `SCENE` | `u8` | 主场景枚举 |
| 4 | `FLAGS` | `u8` | 状态位 |
| 5 | `CRC8` | `u8` | 对 `TURN+SCENE+FLAGS` 做校验 |

## 3. 为什么不用“只有状态位”

如果只保留一个状态位字节，会有两个问题：

- 无法同时表达“方向”和“幅度”
- 路段场景之间很多是互斥主状态，不适合全都挤进 bit 位

所以这里采用：

- `TURN`：主控真正用于控制的连续量
- `SCENE`：当前最主要的路段分类
- `FLAGS`：补充性质、可靠性和附加特征

这种结构比“纯 bit 位”更适合运行态。

## 4. `TURN` 定义

### 4.1 含义

`TURN` 为带符号 8 位整数：

- `0`：不需要修正
- 负数：向左修正
- 正数：向右修正

建议范围：

- `-100 ~ +100`

保留：

- `-128` 作为无效值，不在正常运行中使用

### 4.2 语义建议

`TURN` 不是原始 `center_err`，而是经过 `F103` 本地处理后的“控制建议量”。

例如：

- `-8`：轻微左修
- `-25`：中等左修
- `-60`：强左修
- `+12`：轻微右修
- `+45`：较强右修

这样 `F407` 可直接用：

- `servo_target = servo_mid + k * TURN`

或者：

- 左右轮差速修正

## 5. `SCENE` 主场景定义

`SCENE` 用来表达“当前最主要的道路结构判断”，建议如下：

| 值 | 名称 | 含义 |
| --- | --- | --- |
| `0x00` | `SCENE_NONE` | 无有效场景 / 初始化 |
| `0x01` | `SCENE_STRAIGHT` | 普通直行跟线 |
| `0x02` | `SCENE_SOFT_LEFT` | 普通左转趋势 |
| `0x03` | `SCENE_SOFT_RIGHT` | 普通右转趋势 |
| `0x04` | `SCENE_HARD_LEFT` | 急左转 / 左直角趋势 |
| `0x05` | `SCENE_HARD_RIGHT` | 急右转 / 右直角趋势 |
| `0x06` | `SCENE_CROSS` | 十字区 / 中轴仍压在线上 |
| `0x07` | `SCENE_LEFT_BRANCH` | 左侧出现支路/分叉特征 |
| `0x08` | `SCENE_RIGHT_BRANCH` | 右侧出现支路/分叉特征 |
| `0x09` | `SCENE_T_JUNCTION` | T 字结构或近似 T 字结构 |
| `0x0A` | `SCENE_STOP_BAR` | 停车线 / 大面积横向黑区 |
| `0x0B` | `SCENE_LINE_LOST` | 丢线 |
| `0x0C` | `SCENE_AMBIGUOUS` | 模式复杂、不稳定、待上层兜底 |
| `0x0D~0xFF` | `RESERVED` | 预留 |

## 6. `FLAGS` 状态位定义

`FLAGS` 用于表达补充状态，而不是替代 `SCENE`。

建议定义：

| 位 | 名称 | 含义 |
| --- | --- | --- |
| bit0 | `VALID` | 当前输出有效 |
| bit1 | `LINE_FOUND` | 当前至少找到主线 |
| bit2 | `LEFT_FEATURE` | 左侧存在明显结构特征 |
| bit3 | `RIGHT_FEATURE` | 右侧存在明显结构特征 |
| bit4 | `CENTER_ON_LINE` | 车体对称轴附近仍压主线 |
| bit5 | `WIDE_BLACK` | 出现大面积连续黑区 |
| bit6 | `UNSTABLE` | 当前判定抖动或不稳定 |
| bit7 | `SENSOR_FAULT` | 传感器异常或不可信 |

## 7. `SCENE` 与 `FLAGS` 的关系

这两者不是重复，而是主从关系：

### 7.1 十字

例如十字时：

- `SCENE = SCENE_CROSS`
- `FLAGS`
  - `VALID = 1`
  - `LINE_FOUND = 1`
  - `CENTER_ON_LINE = 1`
  - `WIDE_BLACK = 1`

### 7.2 左直角

例如左直角时：

- `SCENE = SCENE_HARD_LEFT`
- `TURN < 0`
- `FLAGS`
  - `VALID = 1`
  - `LEFT_FEATURE = 1`

### 7.3 普通右转

例如普通右弯时：

- `SCENE = SCENE_SOFT_RIGHT`
- `TURN > 0`
- `FLAGS`
  - `VALID = 1`
  - `LINE_FOUND = 1`

### 7.4 丢线

例如完全丢线时：

- `SCENE = SCENE_LINE_LOST`
- `TURN` 可保留上次值，或输出 `0`
- `FLAGS`
  - `VALID = 1`
  - `LINE_FOUND = 0`
  - `UNSTABLE = 1`

## 8. 为什么这样划分更稳

如果把“十字、直角、普通转向、丢线”全部塞进一个 bit 字段，会导致：

- 主状态互斥关系不清楚
- 同时出现多个 bit 时含义模糊
- 后续扩展困难

采用 `SCENE + FLAGS` 后：

- `SCENE` 负责“当前最主要是什么”
- `FLAGS` 负责“当前还伴随什么性质”

主控解析逻辑会简单很多。

## 9. 主控侧推荐处理方式

`F407` 收到短帧后只做三件事：

1. 校验帧头与 `CRC8`
2. 读取 `TURN`
3. 根据 `SCENE/FLAGS` 选择控制分支

例如：

- `SCENE_STRAIGHT / SOFT_LEFT / SOFT_RIGHT`
  - 正常按 `TURN` 闭环修正
- `SCENE_HARD_LEFT / HARD_RIGHT`
  - 进入急弯控制参数
- `SCENE_CROSS`
  - 交给上层任务逻辑决定“直行/左/右”
- `SCENE_LINE_LOST`
  - 进入丢线恢复逻辑
- `SCENE_STOP_BAR`
  - 减速或停车

## 10. 关于“十字”定义的特别说明

你提到的十字是：

- 小车对称轴仍在一根黑线正上方

这个场景非常重要，所以不建议只靠某个 bit 表达。

建议明确判定为：

- `SCENE = SCENE_CROSS`

同时再用：

- `CENTER_ON_LINE = 1`

表示“虽然进入十字区，但主轴下方仍有主线可依附”。

这样上层可以区分：

- 真正十字可继续决策
- 只是大面积黑区但中心不稳定

## 11. 关于“普通转向”和“直角拐弯”的区别

建议在 `F103` 本地做区分，不要留给 `F407` 去猜。

### 11.1 普通转向

特点：

- 有明显左右偏差
- 主线仍连续
- 跳变数不大
- 中心误差连续变化

输出：

- `SCENE_SOFT_LEFT` 或 `SCENE_SOFT_RIGHT`

### 11.2 直角拐弯

特点：

- 一侧大面积聚集
- 另一侧迅速丢失
- 中心误差迅速饱和
- 常伴随边缘态突出

输出：

- `SCENE_HARD_LEFT` 或 `SCENE_HARD_RIGHT`

## 12. 推荐的第一版判定覆盖范围

建议先把运行态第一版只做到这些就够：

- 直行
- 普通左转
- 普通右转
- 急左 / 左直角
- 急右 / 右直角
- 十字
- 左支路
- 右支路
- 停车线 / 大面积黑区
- 丢线
- 模糊态

这已经足够覆盖大多数基础赛道。

## 13. 推荐的测试样例

### 13.1 普通中线

- `TURN = 0`
- `SCENE = STRAIGHT`
- `FLAGS = VALID | LINE_FOUND | CENTER_ON_LINE`

### 13.2 轻微左偏

- `TURN = -12`
- `SCENE = SOFT_LEFT`
- `FLAGS = VALID | LINE_FOUND`

### 13.3 左直角

- `TURN = -65`
- `SCENE = HARD_LEFT`
- `FLAGS = VALID | LEFT_FEATURE`

### 13.4 十字

- `TURN = 0`
- `SCENE = CROSS`
- `FLAGS = VALID | LINE_FOUND | CENTER_ON_LINE | WIDE_BLACK`

### 13.5 丢线

- `TURN = 0`
- `SCENE = LINE_LOST`
- `FLAGS = VALID | UNSTABLE`

## 14. 最终建议

运行态短帧建议就定成：

```text
AA 55 TURN SCENE FLAGS CRC8
```

原因：

- 比当前调试长帧短很多
- 比“只发 1 字节灰度”更能减轻主控计算
- `SCENE + FLAGS` 足够覆盖你现在提到的典型路段
- 后续仍可继续扩展 `SCENE`，而不需要立即改帧结构
