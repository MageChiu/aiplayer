# Translation Library

`src/libs/translation` is the future home for reusable translation orchestration that can be shared by desktop and later mobile runtimes.

## Layout
- `include/`: future library-facing headers for backend-neutral translation services
- `src/`: future implementation of orchestration, prompt policy, and queueing helpers
- `tests/`: focused checks for backend selection, deduplication, and prompt shaping

## First migration targets
- Backend-neutral request orchestration currently inside `TranslationService`, especially backend selection and duplicate request suppression policy
- Pure prompt-building rules currently embedded in `DesktopLocalTranslationBackend`, such as subtitle translation instruction templates
- Retry, queue, and result-normalization helpers that do not depend on `QSettings`, executable discovery, or platform event loops

## Ownership split
- Keep `localtranslationengine.*` outside this library for now because it owns helper executable discovery, `QProcess` lifecycle, and local model execution wiring
- Keep online provider adapters such as `platform/desktop/onlinetranslationbackend.*` in platform code because they encapsulate provider credentials, network policy, and runtime-specific error mapping
- Keep `shared/facade/translation` as the stable cross-platform consumption surface; `libs/translation` should implement orchestration behind that facade instead of replacing it

## Current migration strategy
- Step 1: introduce backend-neutral orchestration types under `libs/translation` without moving local execution or provider adapters
- Step 2: move pure prompt and queue policy out of `platform/desktop/desktoplocaltranslationbackend.*`
- Step 3: let desktop/mobile adapters depend on the new library while `shared/facade/translation` keeps the stable API surface

## Guardrails
- Do not move helper path resolution, helper process spawning, or model file lookup into `libs/translation`
- Do not move `QSettings` loading into the library; settings remain a platform or product concern
- Do not move platform-specific network clients into the library
