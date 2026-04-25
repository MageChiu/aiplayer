- [x] `src/shared` 已从占位骨架推进到桌面、Android、iOS 可真实消费的共享接口层
- [x] `libs/asr`、`libs/logging`、`libs/model` 已完成第一轮去桌面化，不再直接依赖桌面产品层
- [x] `libs/translation`、`libs/subtitles`、`libs/media` 已具备明确目录与迁移策略，不再停留在 spec 目标层
- [x] `platform/mobile/android` 与 `platform/mobile/ios` 已完成到独立平台目录的物理拆分
- [x] `apps/desktop`、`apps/android`、`apps/ios` 已形成最小可用入口，而不仅是映射计划
- [x] 三端构建脚本已具备外部可执行性，并能对缺失环境给出明确提示
- [x] 兼容转发头已形成清理计划，第二阶段迁移结果与 spec 描述一致

## Task 13 复核结果

- 已清理 `src/core` 下 9 个兼容转发头和 3 份遗留实现文件，迁移后的 `shared` / `libs` 消费点不再依赖这些兼容层
- 已确认 `src/platform/mobile` 不再承载 Android/iOS 真实实现，仅保留 3 个过渡资产，并在 `src/platform/mobile/README.md` 中记录退出条件
- 已确认 `src/shared`、`src/libs`、`src/platform/android`、`src/platform/ios`、`apps/desktop`、`apps/android`、`apps/ios` 均有真实入口或实现文件，与当前 spec 描述一致
- `./scripts/build_macos.sh --debug` 构建通过；`./scripts/build_android.sh --debug --apk` 明确提示缺少 `gradlew` 或 `gradle`；`./scripts/build_ios.sh --debug --simulator` 可生成工程并明确提示缺少 iOS Simulator runtime
