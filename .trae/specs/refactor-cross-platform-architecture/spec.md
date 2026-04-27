# 跨平台 AI 播放器第二阶段适配与收口 Spec

## Why
第一阶段已经完成桌面基线、`apps / platform / shared / libs` 分层原则、首批 `shared`/`libs` 骨架，以及部分低风险物理迁移，但距离“可持续实现跨平台 AI 播放器”还差最后一批关键收口工作。当前主要缺口是：`shared` 仍有占位骨架、`libs` 仍残留桌面依赖、`platform/mobile` 尚未物理拆分、`apps/*` 仍以映射为主而未形成稳定入口、Android/iOS 构建脚本虽已提供但缺少和新目录的最终对齐。

## What Changes
- 将本 spec 的关注点从“建立分层基础”切换到“完成第二阶段适配与收口”
- 继续深化 `shared`，把当前占位骨架补成真实可消费的协议、门面和桥接入口
- 继续深化 `libs`，把首批已迁移能力从桌面/平台依赖中剥离，并补齐 `translation`、`subtitles`、`media` 的迁移策略
- 物理拆分 `platform/mobile/android` 与 `platform/mobile/ios` 到各自长期目录
- 将 `apps/desktop`、`apps/android`、`apps/ios` 从“逻辑映射”推进到“最小可用入口”
- 统一三端构建脚本与外部校验入口，确保外部可独立验证桌面、Android、iOS
- **BREAKING**：后续平台目录、工程入口、include/import 路径会继续发生调整
- **BREAKING**：兼容转发头是阶段性措施，第二阶段需要分批清理

## Impact
- Affected specs: 共享协议能力、共享门面能力、核心能力库去耦、平台目录拆分、平台工程入口、外部构建与校验能力
- Affected code: `src/shared/`、`src/libs/`、`src/platform/mobile/`、未来 `src/platform/android`、`src/platform/ios`、未来 `apps/*`、`scripts/build_*.sh`、`CMakeLists.txt`、Android Gradle、iOS XcodeGen

## ADDED Requirements
### Requirement: Shared 层必须从骨架进入真实可消费状态
系统 SHALL 将 `src/shared` 从“目录和占位门面已建立”的状态推进到“桌面、Android、iOS 可稳定消费的共享协议层”。

#### Scenario: 平台消费 shared
- **WHEN** `desktop`、`android` 或 `ios` 需要读取共享状态、调用共享协议或接入桥接入口
- **THEN** `shared/contracts`、`shared/models`、`shared/facade`、`shared/bridge` 中对应目录必须至少提供一个真实可消费入口
- **THEN** 不得继续仅以 README 或占位类型代替实际接口边界

### Requirement: 已迁入 libs 的能力必须继续去桌面化
系统 SHALL 继续把已迁入 `libs/asr`、`libs/logging`、`libs/model` 的能力从桌面专属依赖中剥离。

#### Scenario: 库能力被其他平台复用
- **WHEN** Android 或 iOS 计划消费 `libs/asr`、`libs/logging` 或 `libs/model`
- **THEN** 这些库不得直接依赖桌面窗口类、桌面路径策略或桌面产品入口
- **THEN** 桌面相关依赖必须通过配置、门面或平台适配提供

### Requirement: Platform 目录必须完成 Android/iOS 物理拆分
系统 SHALL 停止以 `platform/mobile/android` 和 `platform/mobile/ios` 作为长期目录结构，并将其物理迁移到独立平台目录。

#### Scenario: 平台目录重构
- **WHEN** 团队推进 Android 或 iOS 平台集成代码的下一轮迁移
- **THEN** 新代码应进入 `platform/android` 或 `platform/ios`
- **THEN** 旧的 `platform/mobile` 仅可保留少量过渡资产，并需要明确清理计划

### Requirement: Apps 目录必须形成最小可用入口
系统 SHALL 将 `apps/desktop`、`apps/android`、`apps/ios` 从“映射目标”推进为“真实可用的工程入口”。

#### Scenario: 外部校验平台构建
- **WHEN** 外部开发者或 CI 需要验证桌面、Android 或 iOS 构建
- **THEN** 必须能够从平台独立入口启动构建，而不是依赖隐式目录约定
- **THEN** 平台工程入口、平台实现层与共享能力层的边界必须可追踪

### Requirement: 三端构建脚本必须支持外部校验
系统 SHALL 提供桌面、Android、iOS 的统一脚本入口，并在环境不满足时给出明确错误提示。

#### Scenario: 外部执行构建脚本
- **WHEN** 外部执行 `scripts/build_*.sh`
- **THEN** 脚本必须先检查必要依赖和平台环境
- **THEN** 失败提示必须指出缺失项、缺失平台运行时或错误的开发环境状态

## MODIFIED Requirements
### Requirement: 跨平台产品架构
系统 SHALL 从“已建立第一阶段分层基础”的现状，继续推进到“共享协议层、核心能力库、平台目录和平台工程入口都进入可持续演进状态”的架构；第二阶段的重点不再是确认分层方向，而是把剩余适配和迁移落到可执行、可验收、可校验的收口任务上。

#### Scenario: 第二阶段推进
- **WHEN** 团队执行第二阶段适配任务
- **THEN** 每个任务必须明确当前缺口、迁移目标和验收标准
- **THEN** 任何变更仍需遵守桌面基线优先和共享层不反向承载平台特例

## REMOVED Requirements
### Requirement: 第一阶段任务列表即可覆盖后续实现
**Reason**: 第一阶段任务已经完成，继续沿用原任务列表无法准确反映当前剩余适配缺口，也无法指导第二阶段收口工作。
**Migration**: 用新的第二阶段任务列表替代“基础搭建”任务列表，聚焦 shared 深化、libs 去耦、platform 物理拆分、apps 最小入口和构建校验能力。
