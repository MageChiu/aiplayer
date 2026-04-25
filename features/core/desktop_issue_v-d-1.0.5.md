# Desktop Issue v-d-1.0.5

## Issue 类型

- 类型：后续工作拆分 / 五条并行主线
- 来源：Task 12 / 并行工作流设计
- 当前优先级：高

## 目标

将后续工作正式拆为五条并行主线：

1. `desktop`
2. `android`
3. `ios`
4. `shared`
5. `libs`

这五条主线必须能够并行推进，但又不能失去统一约束。因此每条主线都要明确：

- 输入依赖
- 允许改动范围
- 完成门槛
- 与其他主线的阻塞关系
- 是否影响桌面基线

## 总体原则

### 1. 桌面优先仍然是全局约束
- 任何 `shared`、`libs`、`android`、`ios` 的增量，只要可能影响桌面现有能力，都必须先申报桌面影响
- 若影响桌面基线，则必须先通过 `desktop_baseline_v-d-1.0.0.md` 中定义的桌面回归

### 2. `shared` 与 `libs` 是中间能力层，不是产品层
- `shared` 负责共享协议、共享状态、共享模型、桥接门面
- `libs` 负责可复用能力实现
- 两者都不能反向承载平台特例

### 3. 平台主线允许节奏不同
- `desktop` 可以先稳定
- `android` 与 `ios` 可以根据各自平台特性分开推进
- 不要求五条主线按完全相同速度完成

## 五条并行主线定义

### Workstream A: `desktop`

#### 目标
- 稳定桌面能力基线
- 收紧 `platform/desktop`
- 为未来 `apps/desktop` 迁移准备产品入口边界

#### 输入依赖
- `shared` 已稳定的状态模型与协议
- `libs` 已稳定的能力实现
- 当前桌面基线文档和桌面开发规则

#### 允许改动
- `platform/desktop/*`
- `mainwindow.*`
- `settingsdialog.*`
- `ui/desktop/*`
- 未来 `apps/desktop/*`

#### 完成门槛
- 不丢失桌面现有核心能力
- 至少完成桌面 P0 回归
- 与 `platform/desktop` 的边界可被明确描述

#### 主要阻塞
- 受 `shared` 协议变更影响
- 受 `libs` 能力接口变更影响

#### 输出位置
- `features/desktop/*`

### Workstream B: `android`

#### 目标
- 将 Android 收敛为独立的 `platform/android` + `apps/android`
- 明确 Android 原生优先能力
- 明确 Android 对 `shared` / `libs` 的桥接方式

#### 输入依赖
- `shared` 的共享状态与共享协议
- `libs` 的稳定能力入口
- Android 平台能力边界文档

#### 允许改动
- `src/ui/mobile/android/*`
- `src/platform/mobile/android/*`
- 未来 `platform/android/*`
- 未来 `apps/android/*`
- Android Gradle 工程入口映射

#### 完成门槛
- Android 页面层与平台层边界清晰
- Android 原生能力与桥接能力分离清楚
- 不再继续扩大泛化 `platform/mobile` 的依赖

#### 主要阻塞
- 受 `shared` 状态词汇和协议演进影响
- 受 `libs` 能力接口稳定度影响

#### 输出位置
- `features/android/*`

### Workstream C: `ios`

#### 目标
- 将 iOS 收敛为独立的 `platform/ios` + `apps/ios`
- 明确 iOS 原生优先能力
- 明确 iOS 对 `shared` / `libs` 的桥接方式

#### 输入依赖
- `shared` 的共享状态与共享协议
- `libs` 的稳定能力入口
- iOS 平台能力边界文档

#### 允许改动
- `src/ui/mobile/ios/*`
- `src/platform/mobile/ios/*`
- 未来 `platform/ios/*`
- 未来 `apps/ios/*`
- iOS 工程入口映射

#### 完成门槛
- iOS 页面层与平台层边界清晰
- iOS 原生能力与桥接能力分离清楚
- 不再继续扩大泛化 `platform/mobile` 的依赖

#### 主要阻塞
- 受 `shared` 状态词汇和协议演进影响
- 受 `libs` 能力接口稳定度影响

#### 输出位置
- `features/ios/*`

### Workstream D: `shared`

#### 目标
- 稳定共享协议、共享状态、共享数据模型、共享门面
- 作为平台与能力库之间的稳定接缝

#### 输入依赖
- `desktop` 的基线约束
- `android` / `ios` 对共享模型的真实消费需求
- `libs` 当前可提供的能力范围

#### 允许改动
- `src/shared/contracts/*`
- `src/shared/models/*`
- 后续 `src/shared/facade/*`
- 后续 `src/shared/bridge/*`

#### 完成门槛
- 共享模型和协议不再混入平台特例
- 至少覆盖桌面、Android、iOS 三端真实需要的稳定公共词汇
- 任何字段/接口变更都能说明影响范围

#### 主要阻塞
- 被 `desktop` 基线约束
- 同时向 `android` 和 `ios` 暴露消费面，容易成为多方阻塞点

#### 输出位置
- `features/core/*`

### Workstream E: `libs`

#### 目标
- 沉淀真正可复用的核心能力实现
- 不承载平台 UI、平台路径和平台工程逻辑

#### 输入依赖
- `desktop` 当前最稳定的能力实现
- `shared` 所需的门面与数据结构

#### 允许改动
- 后续 `src/libs/asr/*`
- 后续 `src/libs/model/*`
- 后续 `src/libs/logging/*`
- 后续 `src/libs/translation/*`
- 后续 `src/libs/subtitles/*`

#### 完成门槛
- 能力输入、输出、依赖边界清晰
- 不直接依赖桌面窗口类、平台路径实现和产品入口
- 至少有一个平台能稳定消费其能力

#### 主要阻塞
- 受 `shared` 门面定义影响
- 受桌面现有能力抽取难度影响

#### 输出位置
- `features/core/*`

## 依赖关系

### 一级关系
- `desktop` 是全局验收基线
- `shared` 和 `libs` 是中间能力层
- `android` 与 `ios` 消费 `shared` / `libs`

### 二级关系
- `shared` 依赖 `desktop` 基线和多平台真实需求
- `libs` 依赖桌面现有能力抽取结果
- `android`、`ios` 依赖 `shared` 与 `libs` 的稳定接口

### 并行条件
- 只要不修改同一份共享协议或同一份平台入口文件，`desktop/android/ios` 可以并行推进
- `shared` 和 `libs` 的改动若只新增不破坏现有接口，也可与平台主线并行推进

## 变更申报规则

### 任何工作流都必须标注
1. 本次改动属于哪条主线
2. 是否修改 `shared`
3. 是否修改 `libs`
4. 是否影响桌面基线
5. 若影响桌面，需覆盖哪些 P0/P1/P2 验证项

### `shared` / `libs` 额外要求
- 必须在变更说明中列出：
  - 影响 `desktop` 的点
  - 影响 `android` 的点
  - 影响 `ios` 的点
  - 是否需要平台侧同步修改

## 推荐的并行推进顺序

### 第一轮并行
- `desktop`：稳定桌面边界和 `apps/desktop` 入口准备
- `shared`：稳定首批共享状态/协议
- `libs`：抽第一批低风险能力实现

### 第二轮并行
- `android`：对齐 `platform/android` 与 `apps/android`
- `ios`：对齐 `platform/ios` 与 `apps/ios`

### 第三轮并行
- 五条主线围绕统一的 `shared` / `libs` 入口做增量演进

## 结论

从本轮开始，后续工作不再按“单条大任务串行推进”，而是按：

- `desktop`
- `android`
- `ios`
- `shared`
- `libs`

五条主线并行推进。所有主线都必须遵守桌面基线优先和共享层不反向承载平台特例这两个根约束。
