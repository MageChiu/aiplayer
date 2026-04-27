# Shared Bridge Layer

This directory hosts bridge-facing code between shared contracts/models/facades
and platform-specific implementations.

Current status:
- `cpp/` is reserved for C++-first bridge helpers.
- `kotlin/` is reserved for Android bridge surfaces.
- `swift/` is reserved for iOS bridge surfaces.
- `ffi/` is reserved for cross-language FFI shims when needed.

Bridge code must not become a dumping ground for platform-specific business logic.
Platform implementations stay in `platform/<platform>/`.
