# Model Library

`src/libs/model` hosts reusable model-management building blocks.

## Layout
- `include/`: stable library-facing headers
- `src/`: implementation files
- `tests/`: future model-focused checks

## Current scope
- `ModelDownloadService`
- `ModelCatalog` as shared model metadata source
- `ModelManager` as composition root over catalog plus path resolver

## Boundary split
- Shared model metadata: `ModelDescriptor`, `ModelKind`, `ModelCatalog`
- Library logic: fallback selection, lookup by index/url, download orchestration
- Platform path integration: `IModelPathResolver`, currently defaulting to `AppPaths` for desktop compatibility

## Decisions
- `ModelDownloadService` remains download-only and receives `targetFilePath` from callers
- Directory opening, file browser integration, and other desktop UI behavior stay outside `libs/model`
- `shared/facade/model` should consume metadata snapshots and installation state, not desktop path APIs directly

## Next migration targets
- Move `ModelDescriptor` and `ModelCatalog` to `shared` once Android/iOS need direct reuse
- Move desktop-specific resolver wiring from `ModelManager` callers into dedicated platform coordinators
- Replace raw index-based selection with stable model ids in shared-facing APIs
