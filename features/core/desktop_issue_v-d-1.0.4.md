# Desktop Issue v-d-1.0.4

## Issue 类型

- 类型：`apps` 工程入口映射表
- 来源：Task 11 / 平台工程迁移准备
- 当前优先级：中高

## 目标

这份映射表用于回答三个问题：

1. 现在桌面、Android、iOS 的工程入口分别在哪里
2. 未来它们在 `apps/desktop`、`apps/android`、`apps/ios` 中应落到哪里
3. 第一阶段不重写构建系统的前提下，如何建立映射关系并为后续搬迁预留路径

本轮只建立工程入口映射，不要求立即重写全部构建脚本。

## 工程入口映射总原则

- `apps/*` 只承载平台工程入口、构建文件、打包脚本和产品级装配
- `platform/*` 继续承载平台集成逻辑，不随本轮一起整体搬入 `apps/*`
- `shared` 与 `libs` 不承载平台工程入口文件
- 第一阶段先建立“逻辑映射关系”，第二阶段再做物理迁移

## 当前入口到目标入口映射

| 平台 | 当前入口/工程文件 | 目标目录 | 目标归属 | 说明 |
|---|---|---|---|---|
| Desktop | `CMakeLists.txt` | `apps/desktop/CMakeLists.txt` | `apps/desktop` | 未来桌面应用构建入口 |
| Desktop | `src/main.cpp` | `apps/desktop/src/main.cpp` | `apps/desktop` | 桌面应用启动入口 |
| Desktop | `src/mainwindow.*` | `apps/desktop/src/window/` | `apps/desktop` | 主窗口与桌面产品装配 |
| Desktop | `src/settingsdialog.*` | `apps/desktop/src/dialogs/` | `apps/desktop` | 设置对话框属于桌面产品 UI |
| Desktop | `src/ui/desktop/helpdialog.*` | `apps/desktop/src/dialogs/` | `apps/desktop` | 帮助对话框属于桌面产品 UI |
| Desktop | `src/ui/desktop/logwindow.*` | `apps/desktop/src/widgets/` 或 `dialogs/` | `apps/desktop` | 日志窗口属于桌面产品 UI |
| Desktop | `resources/resources.qrc` | `apps/desktop/resources/` | `apps/desktop` | 桌面资源包 |
| Desktop | `scripts/fix_mpv_bundle.sh` | `apps/desktop/packaging/` 或 `scripts/packaging/` | `apps/desktop` | 桌面打包支持脚本 |
| Android | `settings.gradle.kts` | `apps/android/settings.gradle.kts` | `apps/android` | Android 工程根入口 |
| Android | `build.gradle.kts` | `apps/android/build.gradle.kts` | `apps/android` | Android 工程级构建入口 |
| Android | `gradle.properties` | `apps/android/gradle.properties` | `apps/android` | Android 工程级配置 |
| Android | `androidApp/build.gradle.kts` | `apps/android/app/build.gradle.kts` | `apps/android` | Android app 模块入口 |
| Android | `androidApp/src/main/AndroidManifest.xml` | `apps/android/app/src/main/AndroidManifest.xml` | `apps/android` | Android Manifest |
| Android | `androidApp/src/main/res/` | `apps/android/app/src/main/res/` | `apps/android` | Android 资源目录 |
| Android | `src/ui/mobile/android/app/AndroidMainActivity.kt` | `apps/android/app/src/main/java/...` 或保留逻辑映射 | `apps/android` | Android Activity 入口 |
| Android | `src/ui/mobile/android/app/AIPlayerAndroidApp.kt` | `apps/android/app/src/main/java/...` 或保留逻辑映射 | `apps/android` | Android 应用装配入口 |
| iOS | `ios/AIPlayerIOS/project.yml` | `apps/ios/project.yml` | `apps/ios` | iOS XcodeGen 工程入口 |
| iOS | `ios/AIPlayerIOS/` 下工程脚本与配置 | `apps/ios/AIPlayerIOS/` | `apps/ios` | iOS 工程容器 |
| iOS | `src/ui/mobile/ios/App/AIPlayerIOSApp.swift` | `apps/ios/App/` 或逻辑映射到 `apps/ios/src` | `apps/ios` | iOS App 入口 |
| iOS | `src/ui/mobile/ios/App/AppCoordinator.swift` | `apps/ios/App/` | `apps/ios` | 产品装配与应用级协调 |

## 各平台 `apps` 目录最终边界

### `apps/desktop`
- 应包含：
  - 桌面应用入口
  - 桌面窗口与对话框
  - 桌面资源
  - 桌面打包配置与 bundling 脚本
- 不应包含：
  - 桌面播放器实现细节
  - 桌面路径实现
  - 桌面本地翻译 backend
  - 桌面模型目录协调实现

这些内容仍应留在 `platform/desktop`

### `apps/android`
- 应包含：
  - Android Gradle 工程根
  - app 模块入口
  - AndroidManifest、资源、Compose 产品级页面装配
  - Activity / Application 级别入口
- 不应包含：
  - Android 播放器平台实现
  - Android 文件导入/权限/生命周期/后台任务平台实现

这些内容仍应留在 `platform/android`

### `apps/ios`
- 应包含：
  - iOS 工程描述
  - XcodeGen 入口
  - SwiftUI App 入口
  - 产品级页面、主题、资源、工程脚本
- 不应包含：
  - iOS 沙盒实现
  - iOS 文件导入平台实现
  - iOS 原生播放器集成实现

这些内容仍应留在 `platform/ios`

## 第一阶段的推荐映射方式

### 1. 先建立逻辑映射，不立即全量搬目录
- Desktop：先在 spec 和任务中明确 `CMakeLists.txt -> apps/desktop/CMakeLists.txt`
- Android：先明确 `androidApp/` 未来对应 `apps/android/app/`
- iOS：先明确 `ios/AIPlayerIOS/project.yml` 未来对应 `apps/ios/project.yml`

### 2. 保持现有构建可用
- 第一阶段继续允许现有构建入口存在
- 不要求这轮同时修改 Desktop CMake、Android Gradle、iOS XcodeGen 的所有路径

### 3. 为第二阶段预留物理迁移路径
- 先把“产品入口”和“平台实现”在逻辑上分开
- 再逐步把 `apps/*` 真正落地到新目录

## 第一批不建议立即搬迁的内容

### Desktop
- `src/mpvwidget.*`
- `src/platform/desktop/*`

### Android
- `src/platform/mobile/android/*` 当前平台实现
- 任何会影响现有 Gradle sourceSets 的大规模搬迁

### iOS
- `src/platform/mobile/ios/*` 当前平台实现
- 任何会立即打断 XcodeGen 工程生成的路径重写

## 下一步建议

1. 先在任务层确认 `apps/desktop`、`apps/android`、`apps/ios` 的逻辑边界已经成立
2. 第二步再挑最稳妥的入口开始物理迁移：
   - `resources/resources.qrc`
   - `helpdialog.*`
   - `logwindow.*`
   - Android `settings.gradle.kts` / `build.gradle.kts` 的目标落点映射
   - iOS `project.yml` 的目标落点映射
3. 暂不改写桌面主窗口、Android sourceSets、iOS source sources 的实际构建路径，避免在本轮引入构建回归

## 结论

`Task 11` 的当前阶段已经具备可执行的工程入口映射表。后续物理迁移应以这份映射表为准，而不是边迁移边重新定义目录职责。
