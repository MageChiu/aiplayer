# Android Todo v-a-1.0.6

## 当前阶段

本轮目标是把 Android 从“移动端共性层的一个分支”重新定义为独立的 `platform/android` 工作流，并明确哪些能力必须原生优先，哪些能力桥接 `shared/libs`。

## Android 原生优先能力

### 1. 播放器接入
- Android 播放器控制、播放状态回传、预览播放器替换，都属于 `platform/android/player`
- 不要求与桌面播放器暴露同样的内部能力

### 2. 文件导入
- 本地文件选择、URI 读取、持久权限恢复属于 Android 平台能力
- 应归入 `platform/android/files`

### 3. 权限与生命周期
- 存储权限、前后台切换、系统生命周期监听属于 Android 平台能力
- 应归入 `platform/android/permissions` 与 `platform/android/lifecycle`

### 4. 后台任务
- 长任务、模型下载恢复、后台执行策略属于 Android 平台能力
- 应归入 `platform/android/background`

## Android 需要桥接 `shared/libs` 的能力

### 1. 可桥接到 `shared`
- 共享状态词汇
- 共享字幕模型
- 共享翻译设置模型
- 共享应用状态快照

### 2. 可桥接到 `libs`
- ASR 核心能力
- 翻译核心能力
- 模型管理/下载能力
- 日志核心能力

## 当前 Android 代码的首轮归属建议

### 保留在 `apps/android`
- `ui/mobile/android/app/*`
- `ui/mobile/android/components/*`
- `ui/mobile/android/screens/*`
- `ui/mobile/android/theme/*`
- `ui/mobile/android/model/ui/*`

### 回归 `platform/android`
- `platform/mobile/android/player/*`
- `platform/mobile/android/session/*`
- `platform/mobile/android/core/CoreStateBridge.kt`

### 需要二次评审
- `ui/mobile/android/data/local/*`
- `ui/mobile/android/navigation/*`

原因：这部分既有页面态，也有平台态，需要进一步拆分成产品导航和平台会话边界。

## 当前不自然的桌面对齐痕迹

1. 试图让 Android 会话状态完全复制桌面播放器状态结构
2. 试图把移动端播放器能力直接套进桌面风格接口
3. 试图在移动端过早承载桌面特有能力语义

## 下一步待办

1. 将 Android 的播放器、文件导入、权限、生命周期、后台任务定义为独立平台能力树
2. 让 Android 页面只依赖共享模型和 Android 平台集成层
3. 补一版 Android `apps/android` 与 `platform/android` 的边界对照表
