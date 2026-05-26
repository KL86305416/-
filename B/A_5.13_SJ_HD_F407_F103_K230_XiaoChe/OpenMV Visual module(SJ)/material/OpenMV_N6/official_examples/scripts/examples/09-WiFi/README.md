# 09-WiFi — Wi-Fi 例程

Wi-Fi 网络连接相关例程。

## WINC1500（外部 Wi-Fi 模块）

适用于带 WINC1500 外部 Wi-Fi 模块的旧款 STM32 系列 OpenMV 板卡。

| 例程 | 说明 |
|---|---|
| `fw_update.py` | WINC1500 固件更新 |
| `fw_dump.py` | WINC1500 固件导出 |
| `fw_version.py` | 查看 WINC1500 固件版本 |

---

**N6 注意**：N6 内置 Murata Type 1YN Wi-Fi/BT 模块（CYW43439），**不需要** WINC1500 外部模块。本目录中的 WINC1500 例程对 N6 不适用。N6 的 Wi-Fi 网络功能请参考 `08-RPC-Library/36-Web-Servers/` 中的例程，或查阅 N6 quick reference 中的 network 模块文档。
