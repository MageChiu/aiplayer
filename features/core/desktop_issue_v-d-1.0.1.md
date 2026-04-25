# Desktop Issue v-d-1.0.1

## Issue 类型

- 类型：`core` 重分类 / `shared` 与 `libs` 首批拆分清单
- 来源：Desktop 基线重构
- 当前优先级：高

## 目标

基于当前 `src/core` 的真实内容，先完成首轮逻辑归属盘点，回答三个问题：

1. 哪些资产应该进入 `shared`
2. 哪些资产应该进入 `libs`
3. 哪些资产虽然在 `src/core` 中，但本质上仍属于 `platform`

本轮只定义逻辑归属，不要求立即做物理搬迁。

## 首轮拆分总原则

- `shared` 只接收稳定协议、共享状态、共享模型、共享门面
- `libs` 只接收真正可复用的核心能力实现
- 任何直接绑定桌面 UI、Qt Widget、平台路径、helper 打包结构的内容，都不能直接作为共享资产进入 `shared`
- 若一个模块同时混合了共享协议、能力实现和平台细节，则先拆分再迁移

## 当前 `src/core` 首轮归属表

| 当前文件/目录 | 首选归属 | 结论 | 说明 |
|---|---|---|---|
| `shared/models/state/statevocabulary.h` | `shared/models/state` | 进入 `shared` | 纯共享枚举和导航状态词汇，适合做跨端稳定词汇表 |
| `shared/models/state/appstate.h` | `shared/models/state` | 进入 `shared` | 播放/字幕/翻译状态快照属于共享状态模型 |
| `shared/models/app/appevent.h` | `shared/models/app` 或 `shared/contracts/app` | 进入 `shared` | 事件模型可作为共享日志/诊断事件词汇 |
| `shared/models/subtitle/subtitlesegment.h` | `shared/models/subtitle` | 进入 `shared` | 字幕片段数据结构稳定、跨端可复用 |
| `shared/contracts/translation/translationbackend.h` | `shared/contracts/translation` | 进入 `shared` | 这是标准的翻译后端协议接口 |
| `shared/models/translation/translationsettings.h` | `shared/models/translation` | 进入 `shared` | 翻译配置是共享模型，但后续要注意去掉与单平台强耦合的字段 |
| `core/asr/asrservice.*` | `libs/asr` | 倾向进入 `libs` | ASR 是核心能力实现，但 `AsrSettings::load(QSettings&)` 需拆出持久化耦合 |
| `core/logging/logcenter.*` | `libs/logging` | 倾向进入 `libs` | 日志中心是可复用能力，但当前 Qt signal 形式后续可能需要门面层隔离 |
| `core/translation/translationservice.*` | `待拆分：shared/facade + libs/translation + platform` | 混合资产 | 当前同时知道 online、desktop local、mobile local，不宜原样迁移 |
| `core/translation/localtranslationengine.*` | `platform/desktop` 或 `platform/<platform>/translation` | 属于 `platform` | 依赖 helper 路径和本地执行链路，当前本质仍是桌面/平台实现 |
| `core/download/modeldownloadservice.*` | `libs/model` | 倾向进入 `libs` | 下载能力可复用，但需确认是否继续依赖 Qt Network 作为底层实现 |
| `core/model/modelmanager.*` | `待拆分：shared/models + libs/model + platform/paths` | 混合资产 | 目录策略依赖 `AppPaths`，模型元数据和安装判断应拆开 |
| `core/path/apppaths.*` | `platform/<platform>/paths` + `shared/contracts/app` | 属于 `platform` | 路径解析与目录归属本质上是平台能力 |
| `core/path/ipathprovider.h` | `待拆分：shared/contracts/app + platform/<platform>/paths` | 偏 `platform` | 当前接口混合了平台目录职责，尤其含 `torrentsDirectory` 这类平台特征 |
| `core/player/iplayercontroller.h` | `platform/desktop/player` | 属于 `platform` | 暴露 `QWidget *` 且继承 `QObject`，不能作为跨平台共享播放器协议 |

## 第一批建议进入 `shared` 的资产

### 1. 共享状态与数据模型
- `statevocabulary.h`
- `appstate.h`
- `subtitlesegment.h`
- `appevent.h`
- `translationsettings.h` 的稳定字段

### 2. 共享协议
- `translationbackend.h`

### 3. 后续可补充
- 将来可把更稳定的 `ASR` 请求/结果模型、模型元数据、应用生命周期状态继续沉入 `shared`

## 第一批建议进入 `libs` 的资产

### 1. 优先级最高
- `asrservice.*`
- `modeldownloadservice.*`
- `logcenter.*`

### 2. 需要拆分后进入
- `translationservice.*`
- `modelmanager.*`

### 3. 暂不进入 `libs`
- `localtranslationengine.*`
- `apppaths.*`
- `iplayercontroller.h`

## 当前仍应保留在 `platform` 的资产

### 1. 平台路径
- `apppaths.*`
- `ipathprovider.h` 当前形态

### 2. 桌面播放器/桌面 UI 接缝
- `iplayercontroller.h`

### 3. 本地 helper 与平台执行链路
- `localtranslationengine.*`

## 下一步建议

### 对应 Task 4
- 从 `statevocabulary.h`、`appstate.h`、`subtitlesegment.h`、`appevent.h`、`translationbackend.h`、`translationsettings.h` 开始定义首批 `shared` 目录

### 对应 Task 5
- 从 `asrservice.*`、`modeldownloadservice.*`、`logcenter.*` 开始定义首批 `libs`
- 对 `translationservice.*` 和 `modelmanager.*` 做二次拆分设计，再决定最终归属

### 对应 Task 6
- 把 `iplayercontroller.h`、`apppaths.*`、`localtranslationengine.*` 明确收回 `platform/desktop` 的长期边界讨论中

## 结论

当前 `src/core` 不能整体视为未来的共享层。它至少包含三类资产：

1. 可以进入 `shared` 的稳定协议与模型
2. 可以进入 `libs` 的能力实现
3. 仍应回归 `platform` 的平台集成代码

后续迁移必须先按这三类拆开，再决定目录迁移顺序。
