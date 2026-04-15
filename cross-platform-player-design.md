# 跨平台影音播放器 + 实时语音识别双语字幕 — 技术方案

> **目标**：实现一款跨平台（macOS + Windows）影音播放器，支持 AVI/MKV/MP4 等主流格式播放，并在播放过程中实时识别语音、翻译为指定语言、叠加显示双语字幕。

---

## 1. 整体架构

### 1.1 系统架构图

```
┌────────────────────────────────────────────────────────────┐
│                        Qt 6 UI Layer                       │
│  ┌──────────────────────────────────────────────────────┐  │
│  │                    Video Viewport                     │  │
│  │                                                      │  │
│  │        ┌──────────────────────────────────┐          │  │
│  │        │  This is the original sentence.  │  ← 原文  │  │
│  │        │  这是原始的句子。                  │  ← 译文  │  │
│  │        └──────────────────────────────────┘          │  │
│  └──────────────────────────────────────────────────────┘  │
│  [▶ Play] [⏸] [Seek Bar]  [🌐 EN→ZH] [⚙ ASR Settings]    │
└────────────────────────────────────────────────────────────┘
```

### 1.2 内部数据流

```
  Audio Stream ──→ Demuxer ──→ Audio Decoder ──→ Speaker
                                    │
                                    ▼ (PCM 16kHz mono)
                              ┌───────────┐
                              │  ASR Engine│ (Whisper / FunASR)
                              └─────┬─────┘
                                    │ 识别文本 + 时间戳
                                    ▼
                              ┌───────────┐
                              │ Translator │ (离线/在线翻译)
                              └─────┬─────┘
                                    │ 原文 + 译文 + 时间区间
                                    ▼
                              ┌───────────┐
                              │ Subtitle   │
                              │ Renderer   │ (OSD 叠加渲染)
                              └───────────┘
```

### 1.3 线程模型

```
Main Thread (Qt Event Loop)
  ├── UI 渲染 + 字幕叠加 (SubtitleOverlay)
  │
  ├── Demux Thread
  │     └── av_read_frame → 分发 audio/video packet
  │
  ├── Video Decode Thread → 渲染
  │
  ├── Audio Decode Thread → AudioSplitter
  │                           ├── Speaker Output
  │                           └── ASR Buffer (lock-free ring buffer)
  │
  ├── ASR Thread (Whisper.cpp)
  │     └── 每 5s chunk → 识别 → emit segmentReady
  │
  └── Translator Thread
        └── segmentReady → 翻译 → emit bilingualReady → 更新字幕
```

---

## 2. 技术栈选型

### 2.1 总体技术栈

| 层级 | 推荐方案 | 说明 |
|---|---|---|
| **多媒体引擎** | FFmpeg (libavformat + libavcodec + libswscale + libswresample) | 解封装 + 解码的事实标准，覆盖几乎所有格式 |
| **跨平台 UI** | Qt 6 | 最成熟的跨平台桌面 UI 框架 |
| **视频渲染** | OpenGL（兼容两端）/ Metal(macOS) + D3D11(Win) 按平台分发 | Qt 的 QOpenGLWidget 可统一处理 |
| **音频输出** | SDL2 / Qt Multimedia | SDL2 最简单，跨平台零配置 |
| **硬件加速** | macOS: VideoToolbox / Windows: DXVA2 / D3D11VA | FFmpeg 内置支持，通过 `hw_device_ctx` 启用 |

### 2.2 语音识别（ASR）选型

| 方案 | 模型 | 语言 | 离线 | 实时性 | 推荐场景 |
|---|---|---|---|---|---|
| **Whisper.cpp** | OpenAI Whisper | C/C++ | ✅ | ⭐⭐⭐ | **首选**，跨平台、纯 C++ 无依赖 |
| **Faster-Whisper** | CTranslate2 优化版 | Python | ✅ | ⭐⭐⭐⭐ | 速度比原版快 4x，适合后端进程 |
| **FunASR** | 阿里达摩院 Paraformer | Python/C++ | ✅ | ⭐⭐⭐⭐ | 中文识别效果更优 |
| **Vosk** | Kaldi 系列 | C/Python | ✅ | ⭐⭐⭐⭐⭐ | 模型小、延迟低，适合嵌入式 |
| 云端 API | 各厂商 ASR | REST | ❌ | 依赖网络 | 精度最高但需联网 |

**推荐组合**：**Whisper.cpp**（主力，多语言识别强）+ **FunASR**（中文场景备选）

Whisper.cpp 关键特性：
- 纯 C/C++，macOS 支持 Core ML / Metal 加速，Windows 支持 CUDA
- 支持流式分段识别（以约 30s 音频块为单位）
- 输出带 word-level 时间戳，可精确对齐字幕

### 2.3 翻译模块选型

| 方案 | 类型 | 离线 | 质量 | 说明 |
|---|---|---|---|---|
| **CTranslate2 + OPUS-MT** | 离线 NMT | ✅ | ⭐⭐⭐⭐ | Helsinki-NLP 模型，C++ 推理，速度快 |
| **Argos Translate** | 离线 NMT | ✅ | ⭐⭐⭐ | Python，基于 OpenNMT，支持约 30 种语言对 |
| **LibreTranslate** | 自部署 API | ✅ | ⭐⭐⭐ | 本地起一个 HTTP 服务 |
| 云端 API (DeepL/Google) | 在线 | ❌ | ⭐⭐⭐⭐⭐ | 质量最好，需网络 |

**推荐**：CTranslate2 + OPUS-MT 做离线翻译（纯 C++ 可嵌入），联网时可选 DeepL API 增强质量。

### 2.4 字幕渲染选型

| 方案 | 说明 |
|---|---|
| **libass** | ASS/SSA 字幕渲染库，mpv/VLC 都在用，支持复杂样式、双语排版 |
| **自定义 Qt Overlay** | 在视频窗口上叠加 QLabel / QPainter 绘制，灵活度最高 |
| **FFmpeg subtitles filter** | 硬编码烧录，不适合实时场景 |

**推荐**：UI 层用 Qt Overlay 叠加双语字幕（灵活控制样式和动画），底层用 libass 兼容外挂 SRT/ASS 文件。

---

## 3. 播放器方案对比

### 3.1 方案 A：Qt + libmpv（推荐，最快出成品）

将 mpv 作为渲染引擎嵌入 Qt 窗口，自己只做 UI。

```
┌─────────────────────────────┐
│         Qt 6 UI Layer       │  ← 播放控制、列表、字幕、设置
│  ┌───────────────────────┐  │
│  │   libmpv (渲染窗口)    │  │  ← 解码 + 渲染 + 同步全包
│  └───────────────────────┘  │
└─────────────────────────────┘
```

**优势**：
- mpv 已处理好所有平台的硬解、渲染、同步问题
- 开发量最小，专注 UI 即可
- 格式兼容性等同 mpv（几乎全格式）

### 3.2 方案 B：Qt + FFmpeg（自行实现，学习价值高）

自己用 FFmpeg 解码，用 Qt 渲染和播放音频。

```
┌──────────────────────────────────────┐
│              Qt 6 UI                 │
│  ┌────────┐  ┌──────┐  ┌─────────┐  │
│  │ Demuxer│→│Decoder│→│ Renderer │  │
│  │(FFmpeg)│  │(FFmpeg│  │(QOpenGL)│  │
│  └────────┘  └──────┘  └─────────┘  │
│       ↘                              │
│     Audio Decoder → QAudioSink       │
│              ↕                       │
│        A/V Sync (PTS-based)          │
└──────────────────────────────────────┘
```

**核心模块划分**：

```
src/
├── demuxer/          # 解封装线程
│   └── demuxer.cpp   # avformat_open_input → av_read_frame
├── decoder/
│   ├── video_decoder.cpp  # avcodec_send_packet → avcodec_receive_frame
│   └── audio_decoder.cpp  # 同上 + swr_convert 重采样
├── renderer/
│   ├── video_renderer.cpp # QOpenGLWidget + YUV→RGB shader
│   └── audio_renderer.cpp # QAudioSink 输出 PCM
├── sync/
│   └── av_sync.cpp        # 基于音频时钟的同步策略
├── player/
│   └── player_controller.cpp  # 状态机：Play/Pause/Seek/Stop
└── ui/
    └── main_window.cpp    # Qt 控件布局
```

### 3.3 方案对比总结

| 维度 | Qt + libmpv | Qt + FFmpeg |
|---|---|---|
| 开发周期 | 2-3 周 MVP | 1-2 月 |
| 代码量 | 少（专注 UI） | 多（需实现解码/同步/渲染） |
| 格式兼容性 | 极强（mpv 全格式） | 取决于 FFmpeg 编译选项 |
| 可控性 | 中等（受限于 mpv API） | 完全可控 |
| 学习价值 | 中等 | 非常高 |

---

## 4. 核心模块实现

### 4.1 音频流分流与 ASR 输入

```cpp
// 从解码管线中分流 PCM 数据给 ASR
class AudioSplitter {
public:
    // 主输出 → 声卡播放
    // 分支输出 → ASR 输入缓冲区（重采样到 16kHz mono）
    void onAudioFrame(const AudioFrame& frame) {
        // 1. 送声卡
        audioOutput->write(frame);

        // 2. 重采样后送 ASR 缓冲区
        auto resampled = resampler.convert(frame, 16000, 1); // 16kHz mono
        asrBuffer.push(resampled);
    }
};
```

### 4.2 Whisper.cpp 流式识别

```cpp
// ASR 工作线程 — 滑动窗口处理
class ASRWorker : public QThread {
    void run() override {
        whisper_context *ctx = whisper_init_from_file("ggml-medium.bin");

        while (running) {
            // 累积约 5 秒音频后处理（平衡延迟与准确度）
            auto chunk = asrBuffer.popSeconds(5.0);

            whisper_full_params params = whisper_full_default_params(
                WHISPER_SAMPLING_GREEDY);
            params.language = "auto";        // 自动检测语言
            params.token_timestamps = true;  // 词级时间戳

            whisper_full(ctx, params, chunk.data(), chunk.size());

            // 提取识别结果
            int n = whisper_full_n_segments(ctx);
            for (int i = 0; i < n; i++) {
                SubtitleSegment seg;
                seg.text  = whisper_full_get_segment_text(ctx, i);
                seg.start = whisper_full_get_segment_t0(ctx, i); // 10ms 单位
                seg.end   = whisper_full_get_segment_t1(ctx, i);
                seg.lang  = whisper_full_lang_id(ctx);

                emit segmentReady(seg); // 信号送翻译模块
            }
        }
    }
};
```

### 4.3 翻译管线

```cpp
// 翻译工作线程
class TranslatorWorker : public QObject {
    CTranslate2Translator translator; // OPUS-MT 模型

public slots:
    void onSegmentReady(SubtitleSegment seg) {
        // 原文 → 翻译
        QString translated = translator.translate(
            seg.text, seg.lang, targetLang); // e.g., "en" → "zh"

        BilingualSubtitle bilingual;
        bilingual.original   = seg.text;
        bilingual.translated = translated;
        bilingual.startMs    = seg.start * 10;
        bilingual.endMs      = seg.end * 10;

        emit bilingualReady(bilingual);
    }
};
```

### 4.4 双语字幕 OSD 渲染

```cpp
// 字幕叠加层 — 半透明背景 + 双行文本
class SubtitleOverlay : public QWidget {
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);

        // 半透明黑底
        QRect bg = calculateBgRect();
        p.fillRect(bg, QColor(0, 0, 0, 160));

        // 第一行：原文（白色，稍大）
        p.setFont(QFont("Arial", 20, QFont::Bold));
        p.setPen(Qt::white);
        p.drawText(line1Rect, Qt::AlignCenter, current.original);

        // 第二行：译文（浅黄色，稍小）
        p.setFont(QFont("Microsoft YaHei", 18));
        p.setPen(QColor(255, 255, 180));
        p.drawText(line2Rect, Qt::AlignCenter, current.translated);
    }

    // 根据播放进度更新当前字幕
    void onPlayerTimeChanged(qint64 posMs) {
        for (auto& sub : subtitleQueue) {
            if (posMs >= sub.startMs && posMs <= sub.endMs) {
                current = sub;
                update();
                return;
            }
        }
    }
};
```

---

## 5. 跨平台构建

### 5.1 CMake 配置示例（Qt + libmpv）

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyPlayer)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets Multimedia)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED mpv)

add_executable(MyPlayer
    src/main.cpp
    src/mpv_widget.cpp
    src/main_window.cpp
)

target_link_libraries(MyPlayer PRIVATE
    Qt6::Widgets Qt6::OpenGLWidgets
    ${MPV_LIBRARIES}
)
```

### 5.2 平台差异处理

| 差异项 | macOS | Windows |
|---|---|---|
| 硬件解码 | VideoToolbox (`--hwdec=videotoolbox`) | DXVA2 / D3D11VA (`--hwdec=dxva2`) |
| 打包格式 | `.app` Bundle（CMake `MACOSX_BUNDLE`） | `.exe` + NSIS/WiX 安装包 |
| FFmpeg 分发 | Homebrew `brew install ffmpeg` / 静态链接 | vcpkg `vcpkg install ffmpeg` / 预编译二进制 |
| 签名公证 | Apple Developer ID + `codesign` + `notarytool` | 可选 EV 签名 |
| CI/CD | GitHub Actions `macos-latest` | GitHub Actions `windows-latest` |

---

## 6. 可借鉴的开源项目

### 6.1 播放器相关

| 项目 | 地址 | 说明 |
|---|---|---|
| **mpv** | https://github.com/mpv-player/mpv | 最值得学习的现代播放器，可作为 libmpv 嵌入 |
| **FFplay** | https://github.com/FFmpeg/FFmpeg (`fftools/ffplay.c`) | FFmpeg 自带参考播放器，单文件约 3000 行，入门最佳 |
| **ijkplayer** | https://github.com/bilibili/ijkplayer | B站出品移动端播放器，基于 FFplay 改造 |
| **VLC** | https://github.com/videolan/vlc | 全能型播放器，模块化插件架构 |
| **MPC-Qt** | https://github.com/mpc-qt/mpc-qt | Qt + libmpv，MPC-HC 风格 UI |
| **Haruna** | https://github.com/KDE/haruna | KDE 出品，Qt/QML + libmpv |
| **QtAV** | https://github.com/wang-bin/QtAV | 成熟的 Qt + FFmpeg 多媒体框架 |
| **QMPlay2** | https://github.com/zaps166/QMPlay2 | Qt + FFmpeg 播放器，功能完整 |
| **mpv-examples** | https://github.com/mpv-player/mpv-examples | 官方嵌入示例（Qt/SDL/cocoa 等） |

### 6.2 ASR 与翻译相关

| 项目 | 地址 | 说明 |
|---|---|---|
| **Whisper.cpp** | https://github.com/ggerganov/whisper.cpp | ASR 核心引擎，纯 C/C++ |
| **Buzz** | https://github.com/chidiwilliams/buzz | **最接近的参考项目**：Qt + Whisper 实现实时字幕 + 翻译 |
| **Subsai** | https://github.com/abdeladim-s/subsai | Whisper + 翻译自动生成字幕 |
| **CTranslate2** | https://github.com/OpenNMT/CTranslate2 | 高效推理引擎，可同时跑 Whisper 和 OPUS-MT 翻译模型 |
| **OPUS-MT models** | https://github.com/Helsinki-NLP/Opus-MT | 开源翻译模型库，覆盖上百种语言对 |
| **libass** | https://github.com/libass/libass | 字幕渲染引擎 |

---

## 7. 性能优化

| 优化点 | 方案 |
|---|---|
| **ASR 延迟** | Whisper.cpp 使用 `medium` 或 `small` 模型；macOS 开启 Core ML，Windows 开启 CUDA；窗口从 30s 缩到 5s 降低延迟 |
| **翻译延迟** | CTranslate2 量化（int8）OPUS-MT 模型，单句翻译 < 50ms |
| **内存占用** | Whisper medium 模型约 1.5GB，small 约 460MB；可按机器配置自适应选模型 |
| **CPU/GPU 竞争** | 视频解码用硬件加速（VideoToolbox/DXVA2），把 GPU 算力留给 ASR |
| **字幕平滑** | 使用 VAD（语音活动检测）切分句子，避免字幕频繁闪烁；加入淡入淡出动画 |

---

## 8. 实施路线

```
Phase 1（1-2 周）：基础播放器
  └── Qt + libmpv，跑通 macOS / Windows 播放 avi/mkv/mp4

Phase 2（1-2 周）：集成 ASR
  └── 嵌入 whisper.cpp，从音频流分流 PCM → 实时识别
  └── 参考 Buzz 的音频处理逻辑

Phase 3（1 周）：翻译 + 双语字幕
  └── CTranslate2 + OPUS-MT 离线翻译
  └── Qt Overlay 渲染双语字幕

Phase 4（持续优化）：
  └── VAD 优化断句、字幕样式自定义
  └── 支持导出 SRT 双语字幕文件
  └── 可选云端 API（DeepL）提升翻译质量
```

**预估周期**：从零到可用 Demo 约 4-6 周；基于 Buzz + libmpv 整合可缩短到 2-3 周。

---

## 9. 风险与注意事项

| 风险 | 应对 |
|---|---|
| Whisper 模型体积大（medium 约 1.5GB） | 提供模型分级下载，默认使用 small 模型；首次启动引导下载 |
| 实时 ASR 占用大量算力 | 支持用户手动开关 ASR 功能；低配机器自动降级为 tiny 模型 |
| 翻译质量不稳定 | 离线翻译为基础能力，提供云端 API（DeepL）作为高质量选项 |
| 跨平台 UI 一致性 | 使用 Qt 6 统一渲染；避免平台特有控件；CI 双端自动构建 + 截图对比 |
| 音视频同步受 ASR 延迟影响 | 字幕允许 2-5s 延迟是可接受的；ASR 结果按时间戳对齐而非实时弹出 |
| 开源协议合规 | FFmpeg: LGPL/GPL、mpv: GPL v2+、Whisper.cpp: MIT、Qt: LGPL（动态链接） |
