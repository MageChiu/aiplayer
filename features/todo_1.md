# 移动端支持重构清单

## 目标

在不破坏当前 Win/Linux/mac 桌面版功能的前提下，把现有仓库逐步整理成：

- `core`：跨平台可复用能力层
- `ui/desktop`：当前桌面版 UI
- `ui/mobile`：未来 Android / iOS UI
- `platform`：播放器、文件、权限、路径、模型、后台任务等平台适配层

当前仓库已经完成的基础拆分：

- `src/core/logging`
- `src/core/path`
- `src/core/subtitles`
- `src/core/translation`
- `src/ui/desktop`

接下来要做的是把还粘在桌面实现中的能力继续下沉，直到移动端可以只替换 UI 和平台适配层，而不是重写业务逻辑。

## 当前判断

### 已经具备跨平台潜力的部分

- `SubtitleSegment`
- `TranslationSettings`
- `TranslationService`
- `LocalTranslationEngine`
- `AppPaths`
- `LogCenter`

### 仍然强耦合桌面或当前播放器实现的部分

- `MpvWidget`
- `MainWindow`
- `SettingsDialog`
- `TorrentSessionController`
- `whisper.cpp` 调度逻辑
- `FFmpeg` 音频抽取流程
- 本地翻译 helper 运行方式

### 移动端首批高风险点

- `mpv` 在 Android / iOS 的集成方式
- `FFmpeg` 原生库交叉编译和打包
- `whisper.cpp` 与 `llama.cpp` 在移动端的资源占用
- Android / iOS 的文件访问与权限模型
- 后台下载、torrent、长时间任务
- helper 可执行文件方案在移动端不适合继续沿用

## 总体原则

- 先抽核心能力，再做平台 UI
- 先让桌面版继续可构建、可运行，再继续下沉
- 避免直接在现有代码里大量堆 `#ifdef ANDROID` / `#ifdef IOS`
- 移动端第一版优先做 MVP，不追求一次支持全部桌面能力

## 待办清单

### 阶段 1：继续做 core 抽象

- [ ] 抽离 ASR 服务层
  - 目标：把 `whisper` 相关调度从 `MpvWidget` 抽成 `core/asr`
  - 输出建议：
    - `AsrService`
    - `AsrSettings`
    - `AsrResult` / `AsrSegment`

- [ ] 抽离模型管理层
  - 目标：统一管理 ASR 模型、翻译模型的目录、状态、下载源、已安装文件
  - 输出建议：
    - `ModelCatalog`
    - `ModelDescriptor`
    - `ModelManager`

- [ ] 抽离模型下载层
  - 目标：把当前 `SettingsDialog` 里的模型下载流程下沉
  - 输出建议：
    - `ModelDownloadService`
    - `DownloadTask`
    - `DownloadProgress`

- [ ] 抽离播放器外围状态层
  - 目标：把字幕状态、当前播放状态、当前翻译状态、错误状态从 `MpvWidget` 中进一步拆出
  - 输出建议：
    - `PlaybackState`
    - `SubtitleState`
    - `TranslationState`

- [ ] 统一错误与日志事件模型
  - 目标：让桌面和未来移动端都消费同一种日志与状态事件
  - 输出建议：
    - `AppEvent`
    - `LogEvent`
    - `ErrorEvent`

### 阶段 2：建立 platform 层

- [ ] 增加 `src/platform/desktop`
  - 目标：把桌面特有实现从 `core` 中移出
  - 典型内容：
    - 桌面文件选择
    - 桌面目录打开
    - 桌面 helper 启动
    - 桌面 bundle / 动态库处理

- [ ] 增加 `src/platform/mobile`
  - 目标：预留 Android / iOS 共性适配点
  - 典型内容：
    - 移动端文件路径适配
    - 后台任务接口
    - 平台权限接口

- [ ] 抽象路径策略接口
  - 当前已有 `AppPaths`
  - 下一步目标：
    - 桌面路径实现
    - 移动端路径实现
  - 输出建议：
    - `IPathProvider`
    - `DesktopPathProvider`
    - `MobilePathProvider`

- [ ] 抽象模型运行策略
  - 当前桌面本地翻译使用 helper
  - 移动端应改为进程内模型服务
  - 输出建议：
    - `ITranslationBackend`
    - `OnlineTranslationBackend`
    - `DesktopLocalTranslationBackend`
    - `MobileLocalTranslationBackend`

### 阶段 3：播放器能力解耦

- [ ] 抽离播放器接口
  - 目标：不要让 UI 直接依赖 `MpvWidget` 的全部行为
  - 输出建议：
    - `IPlayerController`
    - `PlayerState`
    - `PlayerError`

- [ ] 桌面播放器实现收敛到 `platform/desktop`
  - 目标：把 `mpv + OpenGL + FFmpeg + torrent` 的桌面实现集中管理
  - 输出建议：
    - `DesktopPlayerController`
    - `DesktopSubtitlePipeline`

- [ ] 为移动端预留播放器适配接口
  - Android 可选：
    - `libmpv`
    - `ExoPlayer` + 原生桥接
  - iOS 可选：
    - `AVPlayer`
    - 或 `libmpv`
  - 当前阶段先只定义接口，不急着实现

### 阶段 4：桌面 UI 继续瘦身

- [ ] `MainWindow` 只保留桌面交互编排
- [ ] `SettingsDialog` 只保留桌面设置界面
- [ ] `LogWindow` 只保留桌面诊断展示
- [ ] 移除 UI 层对目录、模型路径、模型下载细节的直接感知
- [ ] UI 层统一通过 service / manager 获取状态和操作入口

### 阶段 5：Android MVP 设计

- [ ] 确定 Android 第一版功能边界
  - 建议先支持：
    - 本地视频播放
    - 字幕显示
    - 在线翻译
  - 暂不支持：
    - torrent
    - 本地 GGUF 翻译 helper
    - 桌面日志窗口

- [ ] 设计 Android UI 层目录
  - 建议：
    - `src/ui/mobile/android`

- [ ] 设计 Android 平台适配层
  - 重点：
    - 存储权限
    - 文件访问
    - 模型缓存目录
    - 后台任务
    - 原生库打包

- [ ] 评估 Android 播放器路线
  - 方案 A：继续 `libmpv`
  - 方案 B：平台播放器 + 字幕/翻译能力复用 `core`

- [ ] 评估 Android 本地翻译路线
  - 不继续沿用 helper 可执行文件方案
  - 改为进程内 `llama.cpp`

### 阶段 6：iOS MVP 设计

- [ ] 确定 iOS 第一版功能边界
  - 与 Android MVP 尽量保持一致

- [ ] 设计 iOS UI 层目录
  - 建议：
    - `src/ui/mobile/ios`

- [ ] 设计 iOS 平台适配层
  - 重点：
    - 沙盒路径
    - 文件导入
    - 原生库签名与打包
    - 后台任务限制

- [ ] 评估 iOS 播放器路线
  - 优先考虑 `AVPlayer`
  - 若要复用现有播放器能力，再评估 `libmpv`

- [ ] 评估 iOS 本地翻译路线
  - 同样不建议 helper 可执行文件方案
  - 优先进程内模型服务

## 推荐实施顺序

1. 继续抽 `core/asr`
2. 抽 `core/model`
3. 抽 `core/download`
4. 抽播放器接口层
5. 把桌面实现收敛进 `platform/desktop`
6. 定义 `platform/mobile` 接口
7. 做 Android MVP
8. 做 iOS MVP

## 当前阶段的明确结论

- 现在还不适合直接开做 Android / iOS UI
- 当前最正确的做法仍然是继续做：
  - `core` 下沉
  - `platform` 分层
  - 桌面 UI 瘦身

只有当这些边界稳定后，移动端才不会变成“重新写一套播放器 + 重新写一套翻译 + 重新写一套模型管理”。

## 验收标准

当满足以下条件时，说明仓库已经基本具备移动端接入条件：

- [ ] `core` 不直接依赖桌面 UI 控件
- [ ] `core` 不直接依赖桌面文件交互行为
- [ ] 播放器实现通过接口接入，而不是直接绑在 `MpvWidget`
- [ ] 模型管理、下载、路径管理都已有独立层
- [ ] 在线翻译 / 本地翻译都有明确后端接口
- [ ] 桌面实现收敛到 `ui/desktop + platform/desktop`
- [ ] 已能为 Android / iOS 单独定义 UI 和平台适配层
