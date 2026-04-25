# ASR Library

`src/libs/asr` hosts reusable speech-to-text capability that can be consumed by
desktop first and later bridged to other platforms.

## Layout
- `include/`: stable library-facing headers
- `src/`: implementation files
- `tests/`: future library-focused checks

## Current scope
- `AsrService`
- `AsrSettings`
- `AsrResult`
- `AsrSegment`
- Library-owned speech-to-text pipeline with platform-injected model resolution

## Platform boundary
- Settings persistence is owned by platform or product code, not by `libs/asr`
- Model lookup is provided through `AsrService::ModelPathResolver`
- Desktop currently wires these concerns in `platform/desktop/desktopasrconfiguration.*`

## Remaining cleanup
- Add cancellation/progress abstractions if Android or iOS need non-desktop orchestration
- Revisit whether model-selection input should switch from `modelIndex` to stable model id
