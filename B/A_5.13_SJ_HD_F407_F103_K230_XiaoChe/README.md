# A_5.13_SJ_HD_F407_F103_K230_XiaoChe

这是当前小车项目的主工作区，包含：

- `F103` 灰度模块工程
- `F407` 主控多个分支工程
- `OpenMV / K230` 视觉模块资料与预留目录
- `dev_logs` 开发日志
- `chat_data` 阶段会话摘要
- `backups` 风险修改前后的备份资料

## 工作区结构

- `F103 grayscale sensor(HD)`
  说明：灰度模块硬件资料与 `STM32F103` 工程
- `F407 main control panel DRV8874`
  说明：旧主控分支，电机驱动为 `DRV8874`
- `F407 main control panel TB6612`
  说明：当前主控主线分支，电机驱动为 `TB6612`
- `OpenMV Visual module(SJ)`
  说明：`OpenMV` 视觉相关资料与后续接入目录
- `K230 Visual module(SJ)`
  说明：`K230` 视觉预留目录
- `dev_logs`
  说明：结构化开发日志
- `chat_data`
  说明：阶段会话摘要与归档索引
- `backups`
  说明：关键改动前后的备份
- `reference_library`
  说明：外部参考资料
- `temp_transfer`
  说明：临时中转目录

## Git 使用约定

详细协作和提交规范见：

- [CONTRIBUTING.md](./CONTRIBUTING.md)

当前仓库默认分支为：

- `main`

## 连接远程仓库

如果要把当前本地仓库接到远程，使用：

```powershell
git remote add origin <REMOTE_URL>
git push -u origin main
```

`<REMOTE_URL>` 可使用：

- `HTTPS`
- `SSH`

## 当前管理原则

这个工作区的版本管理采用“双轨并行”：

- Git 负责版本历史与差异跟踪
- `dev_logs / chat_data / backups` 继续保留项目演进痕迹

这样既能回看代码变化，也能回看开发决策和硬件联调过程。
