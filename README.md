# AIPlayer (智能视频播放器)

AIPlayer 是一款基于 C++ (Qt6) 和 `libmpv` 开发的跨平台现代化视频播放器。它不仅具备主流视频播放器的核心体验，还集成了本地部署的 **Whisper 离线语音识别**和**实时双语字幕翻译**能力，并支持网络流媒体与磁力链接的边下边播。

## ✨ 核心功能

### 🎬 强大的播放能力
- **本地视频播放**: 基于强大的 `libmpv` 内核，支持绝大多数视频格式和编码的硬解播放。
- **在线流媒体**: 支持直接输入 URL 播放 `HTTP/HTTPS`、`HLS (m3u8)`、`RTMP`、`RTSP` 等直播或点播流。
- **磁力链接边下边播**: 粘贴 `magnet:?xt=...` 链接即可通过内置 `libtorrent` 引擎实现 P2P 边下边播，无需额外安装 Node.js。
- **在线视频网站解析**: 搭配 `yt-dlp`，可直接解析并播放 YouTube、Bilibili 等主流视频网站链接。

### 🤖 AI 智能双语字幕
- **内置离线 ASR**: 采用 `whisper.cpp`，完全在本地对视频进行音频提取与语音转文字，保护隐私。
- **一键下载模型**: 在“设置”面板中可一键下载不同精度的 Whisper 模型（Tiny, Base, Small, Medium, Large V3）。
- **实时字幕翻译**: 支持在本地识别出原始字幕后，调用 Google Translate API 进行实时翻译，并在画面上双层渲染“原始+目标”双语字幕。
- **多语言配置**: 支持指定原视频的语音语言（中/英/日/韩/法/德/西/俄等）以提升识别准确率，并自由切换目标翻译语言。

### 🎛️ 完善的播放体验
- **播放控制**: 拖动进度条跳转、0.5x ~ 2.0x 动态倍速调节、音量调节与一键静音。
- **全局快捷键**: 
  - `空格键 (Space)`: 播放 / 暂停
  - `← / → 方向键`: 快退 / 快进 5 秒
  - `↑ / ↓ 方向键`: 音量增大 / 减小
  - `F 键`: 进入 / 退出全屏
  - `Esc 键`: 退出全屏

---

## 🚀 下载与安装 (免编译)

请前往本项目的 [Releases 页面](../../releases) 下载最新版本的预编译包：
- **macOS**: 提供 Intel (`x86_64`) 和 Apple Silicon (`arm64`) 双架构版本。
- **Windows**: 提供免安装的 `.zip` 绿色压缩包，解压即用。

---

## 🛠️ 运行依赖提示

播放器在本地字幕抽取与磁力边播方面已经内置核心能力，但以下功能仍依赖外部环境或网络：
1. **网页视频解析**: 依赖 `yt-dlp` 或 `youtube-dl` 加入系统环境变量。
2. **字幕翻译**: 当前使用在线翻译接口，需要网络可用。
3. **Whisper 模型**: 首次使用需要在设置中下载或手动放入模型目录。

---

## 💻 本地编译指南

如果您希望参与开发或自行编译本项目，请确保您的环境满足以下要求：
- C++17 编译器 (Clang / MSVC)
- CMake 3.16+
- Qt 6.6+
- `libmpv` (开发库，当前按平台处理)

除 `Qt` 与 `libmpv` 外，其余核心 C/C++ 编译依赖已统一收敛到仓库内的 `vcpkg.json` 中，由项目本地 `.deps/vcpkg` 管理，不再要求手工预装 `ffmpeg/libtorrent` 开发库。

### macOS 编译

```bash
# 1. 安装依赖
brew install cmake ninja pkg-config mpv ffmpeg libtorrent-rasterbar
brew install qt@6

# 2. 克隆项目 (包含子模块)
git clone --recursive https://github.com/YourUsername/aiplayer.git
cd aiplayer

# 3. 运行构建脚本（会自动初始化项目内 .deps/vcpkg）
chmod +x scripts/build_macos.sh
./scripts/build_macos.sh --release
```
编译产物位于 `dist/macos/AIPlayer.app`。

### Windows 编译

```powershell
# 1. 克隆项目 (包含子模块)
git clone --recursive https://github.com/YourUsername/aiplayer.git
cd aiplayer

# 2. 运行 PowerShell 构建脚本
# 脚本会自动初始化:
#   .deps\vcpkg
#   .deps\mpv
.\scripts\build_windows.ps1 --release
```

也可以在 `cmd` 中直接运行批处理包装脚本：

```bat
scripts\build_windows.bat --release
```

调试构建：

```powershell
.\scripts\build_windows.ps1 --debug
```

Windows 构建前请确保：
- 已安装 Visual Studio 2022，并包含“使用 C++ 的桌面开发”
- 已安装 Qt 6
- `cmake`、`cl` 可在命令行中使用
- 无需手工安装 `ffmpeg/libtorrent` 开发库，脚本会自动准备项目内依赖

编译产物位于 `dist/windows/aiplayer.exe`。

---

## 📜 许可证 (License)

本项目采用 MIT 许可证。项目中包含的第三方子模块（如 `whisper.cpp`）遵循其各自的开源协议。
