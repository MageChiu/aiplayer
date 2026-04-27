# iOS Todo v-i-1.0.1

## 当前阶段

基于 `features/ios/todo_1.md` 的 MVP 设计，已完成第一轮 iOS UI 壳层搭建。

当前已落地：

- `src/ui/mobile/ios` 的基础目录结构
- `SwiftUI` 根应用与 `AppCoordinator`
- `播放 / 资源 / 设置` 三个一级页面
- 播放器详情页、字幕 overlay、播放器控制组件
- mock ViewModel、主题 tokens、导入入口 sheet

## 下一步待办

- 接入 `AVPlayer` 容器，替换播放器占位视图
- 接入系统 `Document Picker` 和安全访问引用
- 对齐 `core` 中的 `PlaybackState`、`SubtitleState`、`TranslationState`
- 落地 `platform/mobile` 的 iOS 文件路径和导入适配
- 将在线翻译状态从 mock 数据替换为真实 service 状态
- 增补 iPhone 横屏与小屏幕适配

## 风险说明

- 当前代码是 iOS UI 骨架，尚未接入 Xcode 工程与实际运行目标
- 当前仓库主构建仍是桌面 `CMake + Qt`，iOS 代码保持独立，不影响现有桌面端构建
- 真正联调前仍需确定 iOS 工程组织方式和 `AVPlayer` 接入策略
