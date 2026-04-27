# Shared Layer

`src/shared` hosts the cross-platform stable surface of the project.

## Layout
- `contracts/`: stable interfaces and protocol-level entrypoints
- `models/`: shared state snapshots, value objects, and event data
- `facade/`: stable consumption surfaces for platform code
- `bridge/`: bridge-facing entrypoints for C++, Kotlin, Swift, and future FFI

## Rules
- No platform-specific UI types here
- No platform path implementations here
- No desktop-only helper execution logic here
- Platform-specific implementations stay under `src/platform/<platform>/`
