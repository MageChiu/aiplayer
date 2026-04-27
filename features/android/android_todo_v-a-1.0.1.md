# Android Todo v-a-1.0.1

## 版本目标

基于 `features/android/todo_1.md` 的 Android MVP 设计，`v-a-1.0.1` 聚焦完成 Android UI 的第一批基础框架搭建，不直接进入完整播放器实现。

本版本目标：

- 建立 `src/ui/mobile/android` 的基础目录结构
- 明确浏览入口页、播放器页、设置页、启动页的 UI 状态模型
- 建立导航状态与页面 intent 约定
- 预留与 `core`、`platform/mobile` 对接的映射边界
- 保持当前桌面版构建流程不受影响

## 已完成

- 新增 Android UI 框架目录
- 新增 Browser / Player / Settings / Launch 的状态模型草案
- 新增导航骨架与 App Shell 草案
- 新增 Android UI README，说明目录职责与接入顺序

## 下一步

1. 增加 Compose 页面实现骨架
2. 接入系统文件选择器抽象
3. 增加 Android 播放会话控制器接口
4. 补充字幕面板与翻译面板状态映射
5. 建立 Android 模块或子工程接入方案

## 风险

- 当前仓库仍以 Qt/CMake 为主，Android Kotlin UI 尚未进入构建系统
- 播放器选型未最终拍板，播放器容器接口需要继续保持抽象
- 文件访问需要依赖 Android Storage Access Framework，不能沿用桌面路径直读思路
