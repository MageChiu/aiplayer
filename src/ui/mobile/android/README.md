# Android UI Framework

## 目标

该目录用于承载 Android 端的移动 UI 框架，尽量与现有桌面 Qt UI 解耦，并与 `core`、`platform/mobile` 建立清晰的边界。

当前阶段先落“浏览入口 + 播放会话 + 设置”的基础骨架，不直接接入具体 Android 构建系统。

## 设计对齐

本目录对齐 `features/android/todo_1.md` 中的 Android MVP 设计。

其中：

- `Browser` 对应 MVP 文档中的首页 / 内容浏览入口
- `Player` 对应沉浸式播放器页
- `Settings` 对应全局配置页
- `Launch` 对应启动初始化场景

## 目录结构

```text
src/ui/mobile/android/
  app/
  model/ui/
  navigation/
  screens/
    browser/
    launch/
    player/
    settings/
  theme/
```

## 当前代码定位

当前代码以 Kotlin 状态模型、路由约定、Intent reducer、App Shell 为主，目标是：

- 先稳定页面职责边界
- 再接入 Android Compose 页面渲染
- 最后连接 `platform/mobile` 和真实播放器实现

## 接入顺序

1. 建立 Android 模块并引入 Compose
2. 将 `navigation` 与 `app` 连接为真实导航容器
3. 为 `BrowserViewModel` 接入最近媒体与文件选择器
4. 为 `PlayerViewModel` 接入播放器控制器和字幕 / 翻译状态
5. 为 `SettingsViewModel` 接入持久化设置
