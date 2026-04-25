# Android App Module

## 当前状态

该目录承载 `apps/android` 下的 `:app` 模块，当前已经具备：

- `app` Gradle 模块
- `AndroidManifest.xml`
- Compose 依赖配置
- 与 `src/ui/mobile/android`、`src/platform/android` 的源码目录映射
- `src/main/java` 下的 Android 应用入口
- 最近播放本地持久化能力
- 设置本地持久化能力
- `AndroidPlayerController` 播放器接口
- `AndroidPlaybackSessionCoordinator` 会话协调器
- `SharedPreferencesPlaybackSessionStore` 会话持久化实现
- 播放器页字幕 / 翻译 / 样式 / 更多面板骨架

## 当前架构

真实播放器接入前，Android 侧目前分为三层：

1. `screen`：Compose 页面与用户交互
2. `session`：播放会话、字幕偏好、翻译偏好、恢复能力
3. `player`：播放器接口与播放快照

其中 `PlayerScreenRoute` 只依赖会话协调器，不再直接依赖具体播放器实现。

## 运行方式

建议使用 Android Studio 打开 `apps/android` 后执行：

1. 让 IDE 同步 `settings.gradle.kts` 与 `app/build.gradle.kts`
2. 安装 Android SDK 35 与对应 Build Tools
3. 选择 `app` 作为运行模块
4. 在模拟器或真机上运行

## 当前可演示链路

- 启动页可根据最近会话决定是否显示恢复入口
- 首页展示最近播放
- 首页点击 `打开视频` 调起系统文件选择器
- 选择视频后写入最近播放，并创建当前播放会话
- 播放器页通过会话协调器驱动播放状态、字幕偏好、翻译偏好和样式偏好
- 设置页展示全局配置骨架，并可持久化基础设置

## 当前限制

- 仓库暂未内置 Gradle Wrapper
- 真实播放器、真实字幕解析、真实在线翻译尚未接入
- 最近播放进度尚未与真实播放器状态打通
