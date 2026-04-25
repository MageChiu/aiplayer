# Core Feature Intake

这个目录用于承接跨平台基础能力相关的需求、建议和改动说明。

## 文件命名

- Android 基础能力需求：`android_issue_v版本号.md`
- iOS 基础能力需求：`ios_issue_v版本号.md`

版本号规则参考：

- Android：`v-a-1.0.0`
- iOS：`v-i-1.0.0`

## 适用范围

放入这个目录的内容应优先属于跨平台 `core` 能力，例如：

- 后台服务
- 任务调度
- 下载管理
- 模型管理
- 路径策略
- 日志与事件模型
- 播放器接口抽象

以下内容不建议放在这里：

- 仅 Android 端 UI 规划
- 仅 iOS 端 UI 规划
- 单平台特有的页面交互细节

这类需求应分别放到 `features/android` 或 `features/ios`。

## 处理流程

1. 新需求文件进入本目录。
2. 根据 `features/android_develop_rule.md` 和 `features/ios_develope_rule.md` 校验命名与范围是否合理。
3. 判断该需求是否属于跨平台基础能力，是否适合下沉到 `src/core` 或 `src/platform`。
4. 合理则进入开发；不合理则给出调整建议。

## 当前状态

当前目录已建立，等待新的需求文件进入。
