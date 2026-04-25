# iOS Todo v-i-1.0.3

## 当前阶段

在 `v-i-1.0.2` 的基础上，本轮已把 iOS 共享状态桥接迁移到 `platform/mobile/ios`，并新增播放工作区服务统一资源导入与进度保存。

## 下一步待办

1. 将字幕绑定与资源重关联流程接入 `IOSPlaybackWorkspaceService`
2. 为在线翻译、失败重试与离线降级增加平台 service
3. 将播放器会话恢复与后台限制策略继续下沉到 `platform/mobile/ios`
4. 在完整 Xcode 环境中生成工程并完成最小运行验证

## 协作边界

- `ui/mobile/ios`：SwiftUI Screen、ViewModel、展示模型
- `platform/mobile/ios`：状态桥接、沙盒路径、资源仓库、播放工作区、导航/后台能力
- `core`：共享状态词汇、跨端业务规则
