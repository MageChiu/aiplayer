# iOS Todo v-i-1.0.4

## 当前阶段

本轮目标是把 iOS 从“移动端共性层的一个分支”重新定义为独立的 `platform/ios` 工作流，并明确哪些能力必须原生优先，哪些能力桥接 `shared/libs`。

## iOS 原生优先能力

### 1. 播放器接入
- `AVPlayer` 接入、播放状态、画面容器、系统媒体行为属于 iOS 平台能力
- 应归入 `platform/ios/player`

### 2. 文件导入与沙盒
- 文件导入、bookmark、沙盒路径、资源落盘与重定位属于 iOS 平台能力
- 应归入 `platform/ios/files` 与 `platform/ios/sandbox`

### 3. 生命周期与后台限制
- App 生命周期、后台执行限制、恢复策略属于 iOS 平台能力
- 应归入 `platform/ios/lifecycle` 与 `platform/ios/background`

### 4. 签名与工程约束
- 工程生成、签名、bundle 结构、系统权限声明都属于 iOS 平台工程能力

## iOS 需要桥接 `shared/libs` 的能力

### 1. 可桥接到 `shared`
- 共享状态词汇
- 共享字幕模型
- 共享翻译设置模型
- 共享应用状态快照

### 2. 可桥接到 `libs`
- ASR 核心能力
- 翻译核心能力
- 模型管理能力
- 日志核心能力

## 当前 iOS 代码的首轮归属建议

### 保留在 `apps/ios`
- `ui/mobile/ios/App/*`
- `ui/mobile/ios/Components/*`
- `ui/mobile/ios/Screens/*`
- `ui/mobile/ios/ViewModels/*`
- `ui/mobile/ios/Theme/*`
- `ui/mobile/ios/Preview/*`

### 回归 `platform/ios`
- `platform/mobile/ios/CoreStateBridge.swift`
- `platform/mobile/ios/IOSImportedResourceRepository.swift`
- `platform/mobile/ios/IOSNavigationService.swift`
- `platform/mobile/ios/IOSPlaybackWorkspaceService.swift`
- `platform/mobile/ios/IOSSandboxPathService.swift`

### 需要二次评审
- `ui/mobile/ios/Services/IOSFileImportService.swift`
- `ui/mobile/ios/Models/FileImportRequest.swift`

原因：这些文件介于产品层与平台导入层之间，需要进一步判断哪些应进入 `apps/ios`，哪些应回归 `platform/ios/files`。

## 当前不贴合平台的实现方式

1. 试图让 iOS 状态组织过度贴近桌面播放器模型
2. 试图把平台工作区逻辑塞进产品层 ViewModel
3. 试图在共享层中预设 iOS 必须实现与桌面等价的功能集合

## 下一步待办

1. 将 iOS 的播放器、文件导入、沙盒、生命周期、后台限制定义为独立平台能力树
2. 让 iOS 页面只依赖共享模型和 iOS 平台集成层
3. 补一版 iOS `apps/ios` 与 `platform/ios` 的边界对照表
