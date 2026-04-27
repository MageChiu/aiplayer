# Tasks

## 第二阶段：Shared 深化与真实消费面
- [x] Task 1: 将 `src/shared` 从占位骨架推进到真实可消费接口层
  - [x] SubTask 1.1: 为 `shared/contracts/app`、`player`、`subtitle`、`model` 补齐真实接口定义，替换仅作目录占位的入口头
  - [x] SubTask 1.2: 为 `shared/facade/player`、`translation`、`model` 补齐能被桌面、Android、iOS 实际消费的门面接口
  - [x] SubTask 1.3: 为 `shared/bridge/cpp`、`kotlin`、`swift` 明确桥接入口约定，至少提供一个真实桥接入口文件而不是纯 README
  - [x] 验收 1.A: `src/shared/contracts/*`、`src/shared/facade/*`、`src/shared/bridge/*` 不再只有占位说明，存在真实可引用入口
  - [x] 验收 1.B: 桌面构建通过，且不因 `shared` 深化破坏现有桌面回归基线

## 第二阶段：Libs 第一批去耦
- [x] Task 2: 深化 `libs/asr`，剥离桌面化配置与路径依赖
  - [x] SubTask 2.1: 将 `AsrSettings::load(QSettings&)` 从库接口中拆出，改为平台或产品层提供配置
  - [x] SubTask 2.2: 将模型路径解析从 `AsrService` 中抽离为可注入依赖或独立解析接口
  - [x] SubTask 2.3: 评估并记录 `libs/asr` 剩余桌面耦合点，形成后续清理清单
  - [x] 验收 2.A: `libs/asr` 不再直接依赖 `QSettings`
  - [x] 验收 2.B: 桌面 ASR 主链路可继续工作，构建通过且 P0 回归不退化

- [x] Task 3: 深化 `libs/model`，把下载能力与平台目录策略彻底分离
  - [x] SubTask 3.1: 确认 `ModelDownloadService` 仅负责下载本身，不再承担平台目录决策
  - [x] SubTask 3.2: 将 `ModelManager` 拆分为共享模型元数据、库逻辑、平台路径适配三部分的明确任务边界
  - [x] SubTask 3.3: 为后续 `shared/facade/model` 提供清晰的模型门面输入输出定义
  - [x] 验收 3.A: `libs/model` 中的下载能力不直接依赖桌面目录打开或桌面 UI 行为
  - [x] 验收 3.B: 形成 `ModelManager` 拆分后去向清单，并能指导下一轮代码迁移

- [x] Task 4: 深化 `libs/logging`，确认日志能力的长期形态
  - [x] SubTask 4.1: 明确 `LogCenter` 是继续保持 Qt 风格核心，还是需要额外的 shared facade 适配层
  - [x] SubTask 4.2: 保证 `libs/logging` 不直接依赖桌面窗口类与产品 UI
  - [x] SubTask 4.3: 梳理桌面、Android、iOS 将来接入日志能力的统一方式
  - [x] 验收 4.A: `libs/logging` 不直接 include `MainWindow`、`LogWindow`、`SettingsDialog` 等桌面 UI 头
  - [x] 验收 4.B: 形成日志能力长期形态决策，不再停留在“先迁过去再说”

## 第二阶段：Libs 第二批扩展
- [x] Task 5: 建立 `libs/translation` 的目录骨架和首批迁移计划
  - [x] SubTask 5.1: 创建 `src/libs/translation` 的 `include/src/tests` 结构
  - [x] SubTask 5.2: 从 `core/translation` 中筛出可低风险迁移的纯翻译编排逻辑
  - [x] SubTask 5.3: 明确 `localtranslationengine.*`、在线后端适配、shared facade 各自的归属
  - [x] 验收 5.A: `src/libs/translation` 目录结构存在且有首批真实迁移目标
  - [x] 验收 5.B: 平台 helper、本地执行链路没有被误迁入 `libs/translation`

- [x] Task 6: 建立 `libs/subtitles` 与 `libs/media` 的最小可执行策略
  - [x] SubTask 6.1: 创建 `src/libs/subtitles` 和 `src/libs/media` 的最小目录骨架
  - [x] SubTask 6.2: 梳理字幕匹配、字幕缓存、媒体预处理里哪些逻辑适合进入库层
  - [x] SubTask 6.3: 对暂不能迁移的媒体逻辑给出明确延期理由，而不是留空
  - [x] 验收 6.A: `src/libs/subtitles` 与 `src/libs/media` 目录存在且职责清晰
  - [x] 验收 6.B: 不再出现“目录目标已在 spec 中定义，但仓库中完全不存在”的状态

## 第二阶段：Platform 物理拆分
- [x] Task 7: 将 Android 平台实现从 `platform/mobile/android` 物理迁移到 `platform/android`
  - [x] SubTask 7.1: 迁移 `core`、`player`、`session` 目录并更新包名、import、sourceSets
  - [x] SubTask 7.2: 清理旧 `platform/mobile/android` 下的过渡引用
  - [x] SubTask 7.3: 确认 Android 构建脚本和 IDE 索引指向新目录
  - [x] 验收 7.A: Android 平台实现真实落在 `src/platform/android`
  - [x] 验收 7.B: Android 构建脚本可解析新目录，旧目录不再承载真实实现

- [x] Task 8: 将 iOS 平台实现从 `platform/mobile/ios` 物理迁移到 `platform/ios`
  - [x] SubTask 8.1: 迁移 `CoreStateBridge.swift`、`IOSImportedResourceRepository.swift`、`IOSPlaybackWorkspaceService.swift` 等文件到新目录
  - [x] SubTask 8.2: 更新 `project.yml`、`build_ios.sh` 与工程 sources 配置
  - [x] SubTask 8.3: 清理旧 `platform/mobile/ios` 下的过渡引用
  - [x] 验收 8.A: iOS 平台实现真实落在 `src/platform/ios`
  - [x] 验收 8.B: `xcodegen` 工程生成与 iOS 构建脚本能解析新目录

- [x] Task 9: 处理 `platform/mobile` 剩余过渡资产
  - [x] SubTask 9.1: 明确 `mobilecontracts.h` 的最终去向：保留为 shared 入口、拆分或删除
  - [x] SubTask 9.2: 明确 `mobilepathprovider.h` 与 `mobilelocaltranslationbackend.h` 的最终归属
  - [x] SubTask 9.3: 清理不再需要的 `platform/mobile` 过渡层
  - [x] 验收 9.A: `platform/mobile` 不再承载 Android/iOS 真实实现
  - [x] 验收 9.B: 所有保留文件都有明确长期归属说明

## 第二阶段：Apps 入口落地
- [x] Task 10: 将 `apps/desktop` 从映射目标推进到最小真实入口
  - [x] SubTask 10.1: 先迁移低风险桌面产品文件，如 `helpdialog.*`、`logwindow.*`、资源与 packaging 脚本
  - [x] SubTask 10.2: 明确 `mainwindow.*`、`settingsdialog.*`、`mpvwidget.*` 的后续迁移阻塞条件
  - [x] SubTask 10.3: 让桌面构建脚本与新入口结构保持一致
  - [x] 验收 10.A: `apps/desktop` 存在至少一批真实产品文件
  - [x] 验收 10.B: macOS 桌面构建脚本可继续构建且桌面基线不退化

- [x] Task 11: 将 `apps/android` 和 `apps/ios` 从映射表推进到真实工程入口
  - [x] SubTask 11.1: 为 Android 建立真实 `apps/android` 根入口或等价过渡目录
  - [x] SubTask 11.2: 为 iOS 建立真实 `apps/ios` 工程入口或等价过渡目录
  - [x] SubTask 11.3: 让 `build_android.sh`、`build_ios.sh` 与新入口结构对齐
  - [x] 验收 11.A: 外部可以从 `apps/android`、`apps/ios` 的明确入口理解工程边界
  - [x] 验收 11.B: 移动端构建脚本不再完全依赖旧目录隐式约定

## 第二阶段：外部校验与收尾
- [x] Task 12: 统一三端构建与外部校验入口
  - [x] SubTask 12.1: 统一桌面、Android、iOS 脚本的错误提示风格和产物输出位置
  - [x] SubTask 12.2: 为 Android 和 iOS 补齐缺失的环境前置检查与常见失败说明
  - [x] SubTask 12.3: 形成一套外部校验最小执行矩阵
  - [x] 验收 12.A: 三端脚本都能在环境不满足时给出明确可操作的提示
  - [x] 验收 12.B: 外部校验者无需阅读实现细节即可知道如何验证三端构建

- [x] Task 13: 分批清理兼容转发头并完成第二阶段收口验收
  - [x] SubTask 13.1: 梳理当前仍存在的兼容转发头，按风险分批清理
  - [x] SubTask 13.2: 对已完成迁移的 shared/libs/platform/apps 目录做最终一致性检查
  - [x] SubTask 13.3: 执行桌面构建与移动端工程生成/构建验证，形成第二阶段验收结果
  - [x] 验收 13.A: 已迁移模块不再长期依赖兼容转发头存活
  - [x] 验收 13.B: `shared / libs / platform / apps` 的现状与 spec 描述一致
  - [x] 验收 13.C: 桌面构建通过，Android/iOS 构建脚本能够给出可校验结果或明确环境提示
  - [x] 复核记录 13.1: 已删除 `src/core/asr/asrservice.*`、`src/core/download/modeldownloadservice.*`、`src/core/logging/logcenter.*`，以及 `src/core/state/appstate.h`、`statevocabulary.h`、`events/appevent.h`、`subtitles/subtitlesegment.h`、`translation/translationbackend.h` 等仅作转发的兼容头
  - [x] 复核记录 13.2: `src/core/translation/translationsettings.cpp` 已改为直接包含 `shared` 头；仓库内已无 `Compatibility forwarding header during staged migration` 注释残留
  - [x] 复核记录 13.3: `src/platform/mobile` 仅保留 `mobilecontracts.h`、`mobilepathprovider.h`、`mobilelocaltranslationbackend.h` 三个过渡资产，并在 `src/platform/mobile/README.md` 中补齐保留原因与退出条件
  - [x] 复核记录 13.4: 目录一致性检查通过，当前 `src/shared`、`src/libs`、`src/platform/android`、`src/platform/ios`、`apps/desktop`、`apps/android`、`apps/ios` 均存在真实入口文件
  - [x] 复核记录 13.5: 验证结果如下
    - `./scripts/build_macos.sh --debug`：执行成功，桌面工程完成构建
    - `./scripts/build_android.sh --debug --apk`：按预期返回可操作提示，当前环境缺少 `gradlew` 或 `gradle`
    - `./scripts/build_ios.sh --debug --simulator`：可生成 `apps/ios/AIPlayerIOS` 工程，并明确提示当前缺少 iOS Simulator runtime

# Task Dependencies
- Task 2 depends on Task 1
- Task 3 depends on Task 1
- Task 4 depends on Task 1
- Task 5 depends on Task 1
- Task 6 depends on Task 1
- Task 7 depends on Task 1
- Task 7 depends on Task 12
- Task 8 depends on Task 1
- Task 8 depends on Task 12
- Task 9 depends on Task 7
- Task 9 depends on Task 8
- Task 10 depends on Task 12
- Task 11 depends on Task 7
- Task 11 depends on Task 8
- Task 11 depends on Task 10
- Task 12 depends on Task 10
- Task 12 depends on Task 11
- Task 13 depends on Task 2
- Task 13 depends on Task 3
- Task 13 depends on Task 4
- Task 13 depends on Task 5
- Task 13 depends on Task 6
- Task 13 depends on Task 9
- Task 13 depends on Task 12
