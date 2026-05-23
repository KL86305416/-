# 项目日志：全白丢线反向找线

- 时间：2026-05-04 16:34
- 改动范围：巡线丢线恢复逻辑

## 改动内容

- 正常巡线时记录最近一次有效转弯方向。
- 八路灰度传感器全部读到白线时，不再停车。
- 丢线后按照“上一次转弯方向的反方向”低速原地转向找线。
- 一旦任意传感器重新读到黑线，自动退出丢线恢复模式，回到正常巡线控制。
- 新增 `LINE_FOLLOWER_LOST_RECOVERY_SPEED_DUTY`，方便单独调整找线速度。
- 新增 `LINE_FOLLOWER_LOST_RECOVERY_INVERT`，如果实车找线方向相反，可改为 `1U`。

## 调参建议

- 找线太快：降低 `LINE_FOLLOWER_LOST_RECOVERY_SPEED_DUTY`，例如 `65`。
- 找线太慢：提高 `LINE_FOLLOWER_LOST_RECOVERY_SPEED_DUTY`，例如 `100`。
- 全白后转向方向反了：将 `LINE_FOLLOWER_LOST_RECOVERY_INVERT` 从 `0U` 改为 `1U`。
