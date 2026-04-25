# iOS Core Issue v-i-1.0.2

## Issue 类型

- 类型：`core` 共享能力收敛
- 目标平台：iOS 优先，Android 同步对齐
- 当前优先级：高

## 本轮已落地

- 将 iOS `CoreStateBridge` 从 `ui/mobile/ios` 下沉到 `platform/mobile/ios`
- 新增 `IOSPlaybackWorkspaceService`，统一导入资源、解析资源、保存播放进度的工作区编排
- `AppCoordinator` 开始消费 `CoreMobileShellStateSnapshot`，把导航与导入状态对齐到共享词汇

## 当前结论

- iOS 的文件导入、沙盒资源管理与播放工作区已经开始从 UI 层回收到平台层
- UI 层后续应只保留 ViewModel 和 Screen 编排，不再直接扩张资源仓库逻辑
- 若共享状态词汇调整，优先修改 `platform/mobile/ios/CoreStateBridge.swift`

## 下一步建议

1. 将字幕绑定流程接入 `IOSPlaybackWorkspaceService`
2. 为在线翻译与错误降级增加结构化 service
3. 将播放器会话恢复与后台限制策略继续抽象到 `platform/mobile/ios`
4. 在完整 Xcode 环境中生成并验证工程
