# Desktop AIPlayer Develop Rule

1. 版本号规则为 `v-d-1.0.0`，其中 `v-d` 表示 Desktop 版本线，后面的 `1.0.0` 表示主版本号.次版本号.小版本号。
2. 针对桌面版基线能力、桌面回归风险、桌面特有能力边界的需求和建议，统一记录到 `features/desktop` 目录下。
3. 针对会影响共享能力、共享协议、共享模型的桌面诉求，统一记录到 `features/core` 目录下，由 `desktop` 提出 issue，文件名称规则为：`desktop_issue_v版本号.md`。
4. 针对桌面版后续版本规划、桌面架构整理、桌面回归任务，统一记录到 `features/desktop` 目录下，文件名称规则为：`desktop_todo_v版本号.md`。
5. 任何涉及 `shared`、`libs`、`platform`、`apps` 的结构调整，都必须先评估是否影响桌面基线文档 `desktop_baseline_v-d-1.0.0.md` 中定义的能力。
6. 任何 shared/libs 变更，在进入物理目录搬迁或大规模重构前，都必须先完成至少一轮桌面 P0 回归验证。
7. 若改动涉及主播放链路、设置重翻译链路、helper 打包链路或模型下载链路，必须补做对应的 P1/P2 验证。
8. 桌面特有能力可以长期保留在 `platform/desktop` 或未来的 `apps/desktop` 中，不允许为了平台统一而强行下沉到共享层。
9. 如果某项能力是否进入 `shared` 或 `libs` 无法立即确定，优先保留桌面现状和行为，再做逻辑归属评审。
