# iOS Core Issue v-i-1.0.1

## Issue 类型

- 类型：`core` 基础能力修订
- 目标平台：iOS 优先，Android 同步对齐
- 当前优先级：高

## 背景

随着 iOS MVP 从静态 UI 壳层进入“可导入、可播放、可接真实状态”的阶段，现有仓库已经出现一个明显问题：

- `core` 已经有 `TranslationState`
- iOS 仍主要用 `statusText` 驱动展示
- Android 已定义 `TranslationUiStatus`，但语义与 `core` 不一致

这导致同一条翻译链路在不同平台上出现三套状态表达方式，后续一旦接入真实在线翻译、失败重试、离线降级和移动端播放器桥接，UI 与业务层会继续分叉。

## 现状问题

### 1. 状态语义未统一

- `core/TranslationState` 只有 `enabled / mode / provider / targetLanguage / lastError`
- iOS 侧使用自由文本 `statusText`
- Android 侧使用 `Off / Starting / Active / PartialFailure / Error / NoNetwork`

问题在于：

- `core` 无法直接表达“请求中、成功、失败、离线”这些 UI 关键状态
- iOS 与 Android 不能稳定复用同一套状态映射
- 后续桥接到 `AVPlayer` 或移动端服务层时，状态来源和展示会持续散落

### 2. 状态缺少最小共享契约

当前不是完全没有状态桥接，而是：

- 桌面侧已经有 `PlayerState`
- iOS 侧已经有 `CorePlayerStateSnapshot`

但它们没有建立统一的翻译状态契约，导致桥接模型仍处于“各端各写一层”的阶段。

## 本 Issue 的修订目标

本次不一次解决所有移动端接入问题，只先完成第一步：

1. 为翻译链路建立统一状态模型
2. 让 `core`、iOS、Android 使用一致的状态语义
3. 让 UI 从“自由文本状态”迁移到“结构化状态 + 展示文案映射”

## 本次范围

### 目标内

- 统一翻译状态枚举
- 在 `core` 中补齐 `TranslationState.status`
- iOS 从 `statusText` 过渡为结构化状态字段
- Android 将现有 `TranslationUiStatus` 对齐到统一语义
- 首轮打通桌面翻译请求中的状态流转：禁用、请求中、成功、失败

### 目标外

- 不在本轮引入完整的资源元数据抽象
- 不在本轮实现字幕导入/绑定 manager
- 不在本轮接入真实 iOS 在线翻译后端
- 不在本轮设计完整离线翻译降级策略

## 第一阶段落地点

### 主题

统一翻译状态模型

### 建议状态集合

- `disabled`
- `idle`
- `requesting`
- `success`
- `failed`
- `offline`

### 状态语义

- `disabled`：用户关闭翻译能力
- `idle`：翻译能力已开启，但当前没有进行中的请求
- `requesting`：当前有翻译请求正在处理中
- `success`：最近一次翻译请求成功完成
- `failed`：最近一次翻译请求失败
- `offline`：因网络或环境限制进入不可用状态

## 实施建议

### Step 1

在 `core` 中补齐结构化翻译状态：

- 为 `TranslationState` 增加 `status`
- 保留 `lastError` 作为失败补充信息
- 保留 `provider / mode / targetLanguage` 作为上下文，不再承担状态语义

### Step 2

在 iOS 中收敛状态表达：

- 将 `TranslationOptionViewData.statusText` 改为结构化状态
- 通过计算属性输出界面文案
- 让 `CoreTranslationStateSnapshot` 对齐统一状态字段

### Step 3

在 Android 中收敛状态表达：

- 将 `TranslationUiStatus` 改为与 `core` 一致的状态集合
- 保留 Android UI 自己的展示文案，但不再扩散平台特有语义

### Step 4

在桌面翻译链路中补上最小状态流转：

- 翻译关闭时写入 `disabled`
- 发起翻译请求时写入 `requesting`
- 翻译成功时写入 `success`
- 翻译失败时写入 `failed`

## 验收标准

- `core` 已具备结构化翻译状态，而不是只靠 `lastError`
- iOS 不再直接依赖自由文本保存翻译状态
- Android 与 `core` 的翻译状态语义一致
- 桌面翻译链路至少能驱动 `disabled / requesting / success / failed`
- 后续移动端可以直接消费统一状态，再映射为本地 UI 文案

## 后续阶段

当第一阶段完成后，再继续推进：

1. 字幕资源绑定状态模型
2. 资源元数据统一抽象
3. 字幕导入与绑定 manager
4. 移动端播放器状态快照统一契约
