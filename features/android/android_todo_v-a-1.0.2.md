# Android Todo v-a-1.0.2

## 版本目标

`v-a-1.0.2` 作为 `v-a-1.0.1` 之后的下一个小版本，聚焦把当前已经成型的 Android UI 应用骨架继续推进到“可接系统能力、可接真实播放器、可接真实数据”的状态。

## 当前基础

当前仓库已经具备：

- Android Gradle 工程结构
- `androidApp` 应用模块
- `src/ui/mobile/android` 下的 Compose UI 页面骨架
- Browser / Player / Settings / Launch 的基础导航与假数据流

## 下一步开发项

1. 接入 Android Storage Access Framework，替换 Browser 页中的假打开流程
2. 增加真实的最近播放持久化与恢复逻辑
3. 定义 Android 播放控制器接口，并将 Player 页接入真实播放器容器
4. 补齐字幕面板、翻译面板、样式面板的 Compose UI
5. 将翻译状态、字幕状态与 `core` 层事件映射打通
6. 增加错误恢复链路，例如重新授权、文件失效、网络失败重试
7. 增加基础截图、预览或 UI 测试，降低后续回归风险

## 风险与依赖

- 播放器方案仍未最终确定
- `core` 当前仍偏 Qt / 桌面模型，Android 对接层可能需要额外 mapper
- 真实文件访问和权限恢复需要设备或模拟器验证
