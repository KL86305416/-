# 08-RPC-Library — RPC 远程调用例程

OpenMV RPC（Remote Procedure Call）库，支持通过多种物理接口（UART/SPI/I2C/CAN/WiFi）进行远程控制和图像传输。

## 34-Remote-Control（远程控制）

主从架构：一个 OpenMV 作为 Controller（主控端），另一个作为 Remote（远程端）。

| 例程 | 说明 |
|---|---|
| `popular_features_as_the_controller_device.py` | 主控端：调用远程端的常用功能（拍照、检测等） |
| `popular_features_as_the_remote_device.py` | 远程端：响应主控端的功能调用请求 |
| `image_transfer_raw_as_the_controller_device.py` | 主控端（图像传输）：从远程端获取原始图像数据 |
| `image_transfer_raw_as_the_remote_device.py` | 远程端（图像传输）：向主控端发送图像数据 |

## 36-Web-Servers（Web 服务器）

| 例程 | 说明 |
|---|---|
| `rtsp_video_server_lan.py` | RTSP 视频流服务器（有线 LAN）：通过以太网推送实时视频流 |
| `rtsp_video_server_wlan.py` | RTSP 视频流服务器（无线 WLAN）：通过 Wi-Fi 推送实时视频流 |

---

**N6 注意**：N6 具备千兆以太网和 Wi-Fi，非常适合使用 RPC 和 RTSP 视频流功能。RPC 可通过 CAN/UART/SPI/I2C/USB 等多种接口工作。
