# Desktop Issue v-d-1.0.2

## Issue 类型

- 类型：`shared` / `libs` 首批边界定义
- 来源：Desktop 基线重构
- 当前优先级：高

## 目标

在 `desktop_issue_v-d-1.0.1.md` 的拆分清单基础上，进一步明确：

1. 首批 `shared` 应收哪些资产
2. 首批 `libs` 应收哪些资产
3. 哪些资产暂时不能迁入，必须继续留在 `platform`

## 首批 `shared` 边界

### 1. `shared/models/state`
- 接收：`statevocabulary.h`
- 接收：`appstate.h`
- 要求：只保留跨平台稳定的状态词汇、状态快照和枚举
- 不接收：平台 Widget、平台路径、helper 路径、打包路径、平台下载行为

### 2. `shared/models/subtitle`
- 接收：`subtitlesegment.h`
- 要求：只保留字幕片段、时间轴和译文字段等纯数据模型

### 3. `shared/models/translation`
- 接收：`translationsettings.h` 中跨平台稳定字段
- 要求：只保留源语言、目标语言、是否启用、模式、provider 等共享配置
- 不接收：与单个平台打包结构、helper 路径、桌面本地执行链路直接耦合的字段

### 4. `shared/models/app`
- 接收：`appevent.h`
- 要求：作为共享日志/诊断事件模型

### 5. `shared/contracts/translation`
- 接收：`translationbackend.h`
- 要求：保留翻译请求、成功/失败回调、取消协议

### 6. `shared/contracts/player`
- 当前仅定义“共享语义”，不直接迁入 `iplayercontroller.h`
- 原因：当前 `iplayercontroller.h` 绑定 `QObject` 和 `QWidget *`，仍属桌面集成接口

### 7. `shared/contracts/model`
- 本轮先定义模型元数据、安装状态、能力门面的共享语义
- `ModelManager` 当前实现不直接迁入，先等待二次拆分

## 首批 `shared` 禁止项

- 禁止将平台路径实现放入 `shared`
- 禁止将 `QObject + QWidget` 类型接口直接定义为跨平台共享协议
- 禁止将 `torrent`、helper、沙盒、权限等平台差异直接纳入共享模型
- 禁止让 `shared` 承担“为了兼容平台差异而不断膨胀的折中层”

## 首批 `libs` 边界

### 1. `libs/asr`
- 首批候选：`asrservice.*`
- 输入：音频文件路径、ASR 配置、分段回调
- 输出：转录结果、错误信息、字幕分段
- 依赖：Whisper、音频文件、最小配置对象
- 暂不迁移：`QSettings` 读取逻辑，应从 `AsrSettings::load` 中逐步剥离

### 2. `libs/logging`
- 首批候选：`logcenter.*`
- 输入：日志分类、消息、状态键值
- 输出：日志流、状态聚合结果
- 依赖：Qt 容器/信号当前可暂时保留
- 暂不迁移：任何平台窗口或 UI 展示逻辑

### 3. `libs/model`
- 首批候选：`modeldownloadservice.*`
- 第二批候选：`modelmanager.*` 拆分后的一部分
- 输入：下载任务、模型元数据
- 输出：下载进度、完成/失败、安装状态判断
- 依赖：网络、文件写入、模型元数据
- 暂不迁移：平台目录策略与桌面“打开目录”行为

### 4. `libs/translation`
- 当前不直接整体迁入 `translationservice.*`
- 先拆分为：
  - 共享翻译请求/结果模型
  - 可复用翻译编排逻辑
  - 平台本地翻译接入逻辑
- `localtranslationengine.*` 明确不进入首批 `libs`

### 5. `libs/subtitles`
- 本轮不急于创建完整目录
- 先将字幕数据模型收进 `shared/models/subtitle`
- 后续若出现独立字幕匹配、缓存、合并逻辑，再正式建立 `libs/subtitles`

## 当前必须继续留在 `platform` 的内容

### 1. 平台路径与目录
- `apppaths.*`
- 当前形态的 `ipathprovider.h`

### 2. 桌面播放器接缝
- `iplayercontroller.h`

### 3. 本地 helper 执行链路
- `localtranslationengine.*`

### 4. 桌面模型目录与打开目录行为
- `ModelManager` 中依赖 `AppPaths` 的部分
- `DesktopModelCoordinator` 一类桌面集成逻辑

## 第一批可立即执行的迁移切口

### 1. 先进入 `shared`
- `statevocabulary.h`
- `appstate.h`
- `subtitlesegment.h`
- `appevent.h`
- `translationbackend.h`
- `translationsettings.h` 的稳定部分

### 2. 先进入 `libs`
- `asrservice.*`
- `logcenter.*`
- `modeldownloadservice.*`

### 3. 先不要迁
- `iplayercontroller.h`
- `apppaths.*`
- `ipathprovider.h`
- `localtranslationengine.*`
- 未拆分的 `translationservice.*`
- 未拆分的 `modelmanager.*`

## 结论

首批迁移必须遵守三条规则：

1. 先迁纯模型、纯协议、纯能力实现
2. 不迁桌面高风险播放主链路和平台接缝
3. 任何 `shared` 或 `libs` 迁移前都先确认不会破坏桌面基线
