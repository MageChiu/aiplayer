# Desktop Issue v-d-1.0.3

## Issue 类型

- 类型：`platform/mobile` 归属修正
- 来源：跨平台架构重构
- 当前优先级：高

## 问题

当前仓库仍存在一个泛化的 `src/platform/mobile/` 层，它同时承载：

- Android 的会话、播放器、状态桥接
- iOS 的沙盒、导入、工作区、状态桥接
- 一些理论上想做“移动端共性”的协议

这种组织方式适合作为过渡层，但不适合作为长期架构归宿。

## 结论

### 1. `platform/mobile` 不能作为长期层级存在
- Android 和 iOS 在播放器、文件导入、权限、生命周期、后台任务、沙盒等方面差异显著
- 如果长期维持单一 `platform/mobile`，最终会不断在共性层里累积平台分支和折中逻辑

### 2. 长期目标必须拆为
- `platform/android`
- `platform/ios`

### 3. 只有两类内容允许离开平台层
- 稳定共享协议与共享模型，进入 `shared`
- 真正可复用的能力实现，进入 `libs`

## 当前 `platform/mobile` 首轮归属建议

### 进入 `platform/android`
- `platform/mobile/android/core/CoreStateBridge.kt`
- `platform/mobile/android/player/*`
- `platform/mobile/android/session/*`

### 进入 `platform/ios`
- `platform/mobile/ios/CoreStateBridge.swift`
- `platform/mobile/ios/IOSImportedResourceRepository.swift`
- `platform/mobile/ios/IOSNavigationService.swift`
- `platform/mobile/ios/IOSPlaybackWorkspaceService.swift`
- `platform/mobile/ios/IOSSandboxPathService.swift`

### 回流到 `shared`
- `platform/mobile/mobilecontracts.h` 中真正稳定的共享模型与共享契约

### 暂不直接迁移
- `platform/mobile/mobilelocaltranslationbackend.h`
- `platform/mobile/mobilepathprovider.h`

原因：这两个文件需要先判断它们到底是共享门面，还是平台实现残留，再决定去向。

## 迁移规则

1. 先修正逻辑归属，再做物理目录迁移
2. Android 与 iOS 的平台实现不能再以“移动端共性”名义继续堆叠在同一层
3. 凡是含平台生命周期、权限、播放器、沙盒、文件系统差异的实现，默认回归各自平台目录
4. 只有经过确认的稳定协议和数据模型，才允许从 `platform/mobile` 上浮到 `shared`

## 下一步

1. Android 按原生优先能力重新定义 `platform/android`
2. iOS 按原生优先能力重新定义 `platform/ios`
3. 对 `mobilecontracts.h`、`mobilepathprovider.h`、`mobilelocaltranslationbackend.h` 做二次归属评审
