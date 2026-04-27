# Android Todo v-a-1.0.5

## 当前阶段

在 `v-a-1.0.4` 的基础上，本轮已把 Android 的会话层、播放器预览接口和核心状态桥接迁移到 `platform/mobile/android`。

## 下一步开发项

1. 用真实播放器实现替换 `PreviewAndroidPlayerController`
2. 增加 Android 文件导入服务，并将字幕选择结果接入会话层
3. 增加 URI 权限恢复、错误态映射与后台任务入口
4. 将模型缓存目录和权限能力封装为 Android 平台服务
5. 在 Gradle 工程中补最小可运行验证与说明

## 协作边界

- `ui/mobile/android`：Screen、ViewModel、Compose 组件
- `platform/mobile/android`：播放器、会话、文件导入、权限、缓存、后台任务
- `core`：共享状态词汇、跨端业务规则
