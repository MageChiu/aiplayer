# Android Todo v-a-1.0.4

## 版本目标

`v-a-1.0.4` 作为 `v-a-1.0.3` 之后的下一个小版本，聚焦把已经完成的 Android 接口层与播放会话层正式连接到真实播放器实现。

## 当前基础

当前仓库已经具备：

- Android Gradle 工程与 `androidApp` 模块
- Browser / Launch / Player / Settings 四条主路径
- 系统文件选择器接入
- 最近播放与设置的本地持久化
- `AndroidPlayerController` 播放器接口
- `AndroidPlayerSnapshot` 播放快照模型
- `AndroidPlaybackSessionCoordinator` 会话协调器
- `SharedPreferencesPlaybackSessionStore` 会话持久化实现
- 字幕、翻译、样式、更多面板骨架

## 下一步开发项

1. 用真实播放器实现替换 `PreviewAndroidPlayerController`
2. 将播放器真实时间进度、时长、播放状态同步到 `AndroidPlayerSnapshot`
3. 把 `mediaUri` 真实传入播放器容器进行加载
4. 补齐播放器错误态，并映射到会话层与 UI
5. 将外挂字幕选择结果正式绑定到 `AndroidPlaybackSessionCoordinator.attachSubtitle()`
6. 将最近播放进度更新回 `RecentPlaybackStore`
7. 增加重新授权恢复链路，处理 URI 失效场景
8. 增加 Gradle Wrapper 与最小运行说明校验

## 风险与依赖

- 真实播放器路线仍需最终选型
- 真机文件权限与媒体格式兼容仍需验证
- `core` 层字幕与翻译能力接入前，会话层仍以 UI 侧状态为主
