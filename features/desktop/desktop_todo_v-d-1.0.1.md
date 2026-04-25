# Desktop Todo v-d-1.0.1

## 当前阶段

本轮目标不是继续抽象桌面版，而是先收紧 `platform/desktop` 的长期边界，保证桌面功能完整保留，并为后续 `apps/desktop` 迁移做准备。

## `platform/desktop` 当前长期归属

### 1. 播放器集成
- `desktopplayercontroller.*`
- 归属理由：当前负责把桌面播放器能力接到 `MainWindow`，属于典型桌面集成层
- 后续要求：可以继续稳定接口，但不能被误判为跨平台共享播放器协议

### 2. 桌面路径与目录
- `desktoppathprovider.*`
- 归属理由：桌面应用数据目录、模型目录、torrent 目录都属于平台路径能力
- 后续要求：长期保留在 `platform/desktop`，不进入 `shared`

### 3. 桌面模型协调
- `desktopmodelcoordinator.*`
- 归属理由：连接模型管理、下载服务和桌面“打开目录”行为，是桌面集成编排
- 后续要求：可继续拆细，但仍属于桌面平台层

### 4. 桌面在线翻译适配
- `onlinetranslationbackend.*`
- 归属理由：虽然实现在线翻译协议，但当前依赖桌面 Qt 网络栈与桌面运行时形态
- 后续要求：协议保留在 `shared`，该适配实现保留在 `platform/desktop`

### 5. 桌面本地翻译适配
- `desktoplocaltranslationbackend.*`
- 归属理由：显式依赖桌面本地执行链路与 helper
- 后续要求：必须保留在 `platform/desktop`

## 当前应保留在桌面产品/UI 层的内容

### 1. 主窗口与交互装配
- `mainwindow.*`
- 归属：未来 `apps/desktop/src/window`
- 说明：主窗口负责按钮、菜单、快捷键、窗口状态、桌面交互装配，不属于共享层

### 2. 设置窗口
- `settingsdialog.*`
- 归属：未来 `apps/desktop/src/dialogs`
- 说明：设置页面属于桌面产品 UI，但其背后依赖的模型下载、目录打开、平台能力应继续委托 `platform/desktop`

### 3. 帮助与日志窗口
- `ui/desktop/helpdialog.*`
- `ui/desktop/logwindow.*`
- 归属：未来 `apps/desktop/src/dialogs` 或 `apps/desktop/src/widgets`
- 说明：这类窗口是桌面产品层，不应迁入共享层

## 当前不应从桌面侧抽走的内容

### 1. `QWidget *` 视频输出接缝
- 当前 `IPlayerController` 暴露 `QWidget *videoOutputWidget()`，这说明它仍服务于桌面产品层
- 在没有新的跨平台播放器协议前，不应强行视为共享播放器接口

### 2. helper 执行链路
- 桌面本地翻译 helper 的路径发现、执行与打包验证，必须继续按桌面能力管理

### 3. 桌面目录打开行为
- “打开模型目录”“打开翻译模型目录”是明确的平台能力，不应纳入 `shared`

## 当前接缝规则

### 1. `apps/desktop` 与 `platform/desktop`
- `apps/desktop` 负责窗口、页面、按钮、快捷键、产品装配
- `platform/desktop` 负责播放器、路径、helper、模型目录、桌面平台能力

### 2. `platform/desktop` 与 `shared/libs`
- `platform/desktop` 可以消费 `shared` 协议与 `libs` 能力
- `shared/libs` 不应直接依赖 `MainWindow`、`SettingsDialog` 或其他桌面窗口类

### 3. 回归门槛
- 只要改动 `desktopplayercontroller.*`、`desktopmodelcoordinator.*`、`desktoppathprovider.*`、`mainwindow.*`、`settingsdialog.*` 中任意一项，就必须至少完成桌面 P0 回归

## 下一步待办

1. 将 `IPlayerController` 的桌面接缝属性在规格中继续固定，避免误迁到 `shared`
2. 为 `mainwindow.*`、`settingsdialog.*`、`ui/desktop/*` 规划未来 `apps/desktop` 目录落点
3. 在后续物理迁移前，先稳定 `platform/desktop` 与桌面 UI 的调用边界
