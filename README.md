# AIPlayer

AIPlayer 是一个面向 macOS / Windows 的跨平台 AI 视频播放器工程骨架。当前版本为 MVP 第一版，已打通桌面应用入口、主窗口、基础播放器封装、本地视频打开与播放流程，并为字幕识别、翻译、配置管理预留了清晰接口。

## 当前能力

- 基于 PySide6 的桌面主窗口
- 基于 python-mpv / libmpv 的播放器封装
- 支持打开本地视频文件并播放
- 提供播放器状态栏与基础控制按钮
- 预留字幕识别（ASR）接口与占位实现
- 预留翻译接口与占位实现
- 提供跨平台运行时适配点（macOS / Windows）
- 提供 YAML 配置文件与应用配置加载逻辑

## 目录结构

```text
.
├── ARCHITECTURE.md
├── README.md
├── pyproject.toml
├── requirements.txt
├── config
│   └── default.yaml
└── aiplayer
    ├── __init__.py
    ├── app
    ├── asr
    ├── config
    ├── models
    ├── player
    ├── subtitle
    ├── translate
    ├── ui
    └── utils
```

## 环境要求

- Python 3.11+
- 已安装 `libmpv`
- macOS 或 Windows

### macOS 准备 libmpv

可通过 Homebrew 安装：

```bash
brew install mpv
```

### Windows 准备 libmpv

建议：

1. 安装 mpv 或下载 `libmpv` 对应动态库。
2. 将 `mpv-2.dll` 所在目录加入系统 PATH，或在应用配置中指定路径。

## 安装依赖

推荐使用虚拟环境：

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

或：

```bash
pip install -e .
```

## 启动方式

### 方式一：模块启动

```bash
python -m aiplayer.app.main
```

### 方式二：脚本启动

```bash
aiplayer
```

## MVP 使用说明

1. 启动应用。
2. 点击“打开视频”。
3. 选择本地视频文件。
4. 应用会加载并开始播放。
5. 可使用“播放 / 暂停 / 停止”按钮进行基础控制。

## 当前限制

- 依赖本机已正确安装 `libmpv`
- 当前 ASR / 翻译为占位实现，尚未接入真实模型推理
- 当前未实现字幕生成与动态挂载，仅保留清晰接口
- 当前未做安装包与分发配置

## 下一步建议

1. 接入真实的音频抽取与 ASR 流程。
2. 实现字幕文件生成与播放器字幕挂载。
3. 增加任务线程与后台处理队列，避免 AI 任务阻塞 UI。
4. 增加设置面板，支持模型、翻译引擎、缓存目录配置。
5. 补充 macOS / Windows 打包脚本。
