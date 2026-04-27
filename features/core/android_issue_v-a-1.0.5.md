# Android Core Issue v-a-1.0.5

## Issue 类型

- 类型：`core` 共享能力收敛
- 目标平台：Android 优先，iOS 同步对齐
- 当前优先级：高

## 本轮已落地

- 将 Android `CoreStateBridge` 从 `ui/mobile/android` 下沉到 `platform/mobile/android/core`
- 将 Android 播放会话、会话持久化、预览播放器接口统一下沉到 `platform/mobile/android`
- 将 Android Gradle `sourceSets` 显式拆分为 `ui/mobile/android` 与 `platform/mobile/android`

## 当前结论

- Android UI 已不再直接承载会话层与播放器适配层的源码目录
- 后续真实播放器、权限、文件导入、模型缓存都应继续落在 `platform/mobile/android`
- 若共享语义变更，优先修改 `core/state` 与 `platform/mobile/android/core`

## 下一步建议

1. 接入真实播放器实现替换 `PreviewAndroidPlayerController`
2. 增加 Android 文件导入服务与 URI 权限恢复链路
3. 为模型缓存目录和后台任务增加平台服务抽象
4. 将字幕绑定和最近播放进度更新继续从 UI 编排中抽离
