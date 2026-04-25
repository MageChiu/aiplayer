# Logging Library

`src/libs/logging` hosts reusable logging and runtime status aggregation.

## Layout
- `include/`: stable library-facing headers
- `src/`: implementation files
- `tests/`: future logging-focused checks

## Current scope
- `LogCenter`

## Long-term shape
- Keep `LogCenter` as the Qt-based logging core for the current desktop-first runtime
- Do not couple `libs/logging` to desktop widgets, windows, or product dialogs
- Add a shared facade adapter only when Android or iOS require a non-Qt consumption surface

## Cross-platform access plan
- Desktop: consume `LogCenter` directly and bridge updates into `LogWindow` or status UI
- Android/iOS: introduce platform adapters that subscribe to logging events and map them into native observable state
- Shared layer: own vocabulary and snapshot contracts if cross-platform log querying becomes a product requirement

## Current assessment
- `libs/logging` does not include `MainWindow`, `LogWindow`, or `SettingsDialog`
- Remaining coupling is runtime-level Qt dependency, not desktop UI dependency
