# AI小智 (AIXiaoZhi)

<div align="center">

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v4.1+-blue.svg)](https://github.com/espressif/esp-idf)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-v1.0.1-orange.svg)](https://github.com/abookkkk/aixiaozhi/releases)

基于 ESP32 的智能语音助手项目

[English](README_EN.md) | 简体中文

</div>

## 📖 项目简介

AI小智是一个基于 ESP32 平台开发的智能语音助手系统，集成了语音识别、语音合成、LCD显示、WebSocket通信等功能。通过唤醒词激活后，可以进行语音交互，实现智能问答、IoT设备控制等功能。

## ✨ 主要特性

- 🎤 **离线语音唤醒** - 集成 ESP-SR 语音识别引擎，支持离线唤醒
- 🔊 **音频编解码** - 支持实时音频采集、编码和播放
- 📱 **LVGL图形界面** - 基于 LVGL v9.4 的现代化UI界面
- 🌐 **WiFi配网** - 支持二维码配网和SmartConfig配网
- 🔗 **WebSocket通信** - 实时双向通信，支持云端AI服务对接
- 🎛️ **按键控制** - ADC按键支持单击、长按等多种交互方式
- 🏠 **IoT设备管理** - 支持智能家居设备状态查询和控制

## 🛠️ 硬件要求

### 支持的芯片
- ESP32
- ESP32-S2
- ESP32-S3
- ESP32-C2/C3/C5/C6
- ESP32-H2
- ESP32-P4

### 推荐开发板
- ESP32-S3-DevKitC-1
- ESP32-S3-Korvo-2（音频开发板）
- 其他支持PSRAM的ESP32系列开发板

### 外设要求
- LCD显示屏（支持RGB/SPI接口）
- 麦克风（I2S/ADC接口）
- 扬声器/音频输出
- ADC按键（可选）

## 📦 项目结构

```
aixiaozhi/
├── main/
│   ├── adc_button/          # ADC按键驱动
│   ├── audio/               # 音频模块
│   │   ├── xiaozhi_audio.c        # 音频采集与播放
│   │   ├── xiaozhi_audio_codec.c  # 音频编解码
│   │   ├── xiaozhi_audio_decode.c # 音频解码
│   │   └── xiaozhi_sr.c           # 语音识别
│   ├── connect/             # 网络连接模块
│   │   ├── xiaozhi_wifi.c         # WiFi连接管理
│   │   ├── xiaozhi_active.c       # 设备激活
│   │   └── xiaozhi_ws.c           # WebSocket通信
│   ├── LCD/                 # 显示模块
│   │   ├── xiaozhi_lcd.c          # LCD驱动
│   │   └── xiaozhi_lvgl.c         # LVGL界面
│   ├── iot_data/            # IoT数据管理
│   ├── common/              # 公共组件
│   └── main.c               # 主程序入口
├── components/              # 自定义组件
├── managed_components/      # ESP组件管理器依赖
├── build/                   # 编译输出目录
├── sdkconfig               # ESP-IDF配置文件
└── CMakeLists.txt          # CMake构建配置
```

## 🚀 快速开始

### 1. 环境准备

安装 ESP-IDF 开发环境（需要 v4.1 或更高版本）：

```bash
# 克隆 ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# 安装依赖并配置环境
./install.sh
. ./export.sh
```

### 2. 克隆项目

```bash
git clone https://github.com/abookkkk/aixiaozhi.git
cd aixiaozhi/aixiaozhi
```

### 3. 配置项目

```bash
idf.py menuconfig
```

在配置菜单中可以设置：
- 串口配置
- WiFi配置
- 音频参数
- LCD显示参数
- PSRAM配置

### 4. 编译与烧录

```bash
# 编译项目
idf.py build

# 烧录固件
idf.py -p PORT flash

# 查看串口输出
idf.py -p PORT monitor
```

将 `PORT` 替换为实际的串口号，如 `COM3` (Windows) 或 `/dev/ttyUSB0` (Linux)。

### 5. 配网

首次使用需要配置WiFi：

**方法一：长按按键重置WiFi**
1. 长按 SW3 按键
2. 设备进入配网模式
3. LCD显示配网二维码

**方法二：使用配网工具**
1. 使用ESP配网APP扫描二维码
2. 输入WiFi账号密码
3. 等待连接成功

## 🎮 使用说明

### 按键功能
- **SW2 单击** - 自定义功能
- **SW3 单击** - 自定义功能  
- **SW3 长按** - 重置WiFi配网

### 语音交互流程

1. **唤醒** - 说出唤醒词激活设备
2. **等待提示** - 设备播放提示音，准备接收指令
3. **语音输入** - 说出你的指令或问题
4. **处理反馈** - 设备通过WebSocket发送到云端AI服务处理
5. **语音播报** - 接收云端返回的语音并播放

### 工作流程

```
录音 → 语音识别 → 编码 → WebSocket → 云端AI
                                        ↓
扬声器 ← 解码 ← WebSocket ← 音频流 ← 云端AI
```

## 📚 依赖组件

项目使用了以下ESP组件（通过ESP组件管理器自动管理）：

- `espressif/button` ^4.1.5 - 按键驱动
- `espressif/qrcode` ^0.1.0 - 二维码生成
- `espressif/esp_jpeg` >=1.0.2 - JPEG图像解码
- `lvgl/lvgl` ^9.4.0 - 图形库
- `espressif/esp_lvgl_port` ^2.7.0 - LVGL移植层
- `78/xiaozhi-fonts` ^1.5.5 - 中文字体
- `espressif/esp_codec_dev` ^1.5.4 - 音频编解码设备
- `espressif/esp-sr` ^2.3.0 - 语音识别
- `espressif/esp_audio_codec` ^2.4.0 - 音频编解码
- `espressif/esp_websocket_client` ^1.6.0 - WebSocket客户端

## 🔧 开发指南

### 添加自定义功能

1. 在相应模块目录下创建新的 `.c` 和 `.h` 文件
2. 在 `main/CMakeLists.txt` 中添加源文件
3. 在 `main.c` 中注册回调函数
4. 实现功能逻辑

### 修改UI界面

编辑 `main/LCD/xiaozhi_lvgl.c`，使用LVGL API创建和修改UI元素。

### 自定义WebSocket协议

修改 `main/connect/xiaozhi_ws.c` 中的收发处理函数：
- `xiaozhi_ws_text_process_cb()` - 文本消息处理
- `xiaozhi_ws_binary_process_cb()` - 二进制消息处理

## 📝 版本历史

- **v1.0.1** (2026-03) - 优化性能，修复已知问题
- **v1.0.0** (2025-12) - 首个正式版本发布
- 语音识别模块集成
- 音频播放和WiFi连接
- LVGL界面和二维码配网
- 按键和WiFi模块初始化

## 🤝 贡献

欢迎提交Issue和Pull Request！

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 提交 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 🙏 致谢

- [ESP-IDF](https://github.com/espressif/esp-idf) - Espressif官方开发框架
- [LVGL](https://github.com/lvgl/lvgl) - 轻量级嵌入式图形库
- [ESP-SR](https://github.com/espressif/esp-sr) - Espressif语音识别引擎

## 📮 联系方式

- GitHub Issues: [提交问题](https://github.com/abookkkk/aixiaozhi/issues)
- 项目主页: [https://github.com/abookkkk/aixiaozhi](https://github.com/abookkkk/aixiaozhi)

---

<div align="center">
Made with ❤️ by 信仰之名
</div>
