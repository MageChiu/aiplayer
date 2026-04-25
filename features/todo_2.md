# 后续建议记录

## 本轮新增建议

- 为 `libtorrent` 增加可选构建开关
  - 当前桌面端配置直接依赖 `libtorrent-rasterbar`，一旦本机缺少该依赖，连基础播放器构建都无法继续。
  - 建议增加类似 `AIPLAYER_ENABLE_TORRENT` 的 CMake 选项，让桌面 MVP 在缺少 torrent 依赖时仍可构建、仍可播放本地文件和普通网络流。

- 继续把 torrent 逻辑从 `MpvWidget` 挪出
  - 这次已经把播放器控制入口抽到 `IPlayerController`，但 `TorrentSessionController` 仍然内嵌在 `MpvWidget`。
  - 建议下一步收敛到 `src/platform/desktop`，避免播放器渲染、字幕、torrent 生命周期继续耦合。

- 给 `AsrService` 增加取消能力
  - 当前桌面端仍由 `MpvWidget` 控制线程停止，`AsrService` 本身还没有显式取消接口。
  - 后续若要做移动端后台任务或更细粒度的任务管理，建议补充取消 token / 中断回调。

- 给模型下载服务补充统一任务状态
  - `ModelDownloadService` 已经抽出，但现在仍以单任务模式工作。
  - 后续可以补 `DownloadTaskId`、队列、失败重试与持久化状态，为移动端下载管理做准备。

- 给播放器接口补一个轻量 smoke test
  - 现在 `MainWindow` 已改为依赖 `IPlayerController`，适合增加桌面端的最小回归验证。
  - 建议后续补一个不依赖真实媒体解码的假控制器测试，验证播放按钮、进度条、静音和设置回调的编排逻辑。
