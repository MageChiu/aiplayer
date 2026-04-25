# Subtitles Library

`src/libs/subtitles` is the planned home for reusable subtitle timeline and text-processing logic that should not stay embedded in widgets or platform sessions.

## Layout
- `include/`: future public headers for subtitle timeline, matching, and cache abstractions
- `src/`: future implementation of subtitle search, merge, and cache helpers
- `tests/`: focused checks for timeline lookup and cache behavior

## Candidate library logic
- Subtitle timeline lookup by playback position, which is currently repeated inline around `m_subtitles` traversal in `MpvWidget`
- Segment normalization, merge, sorting, and retranslation reset helpers that operate only on `SubtitleSegment` collections
- Subtitle cache keys and in-memory snapshot helpers once desktop and mobile need the same lookup semantics

## Keep outside for now
- UI overlay rendering and native subtitle presentation on desktop, Android, and iOS
- File import flows and persistence shells that are currently platform-specific
- Any player event subscription that still depends on mpv callbacks, Qt widgets, or mobile session stores

## Current migration strategy
- Step 1: extract a backend-neutral `SubtitleTimeline` that answers current-segment and reset/retranslate queries over `SubtitleSegment`
- Step 2: introduce a lightweight subtitle cache abstraction only when at least two runtimes need shared caching semantics
- Step 3: keep `shared/models/subtitle/subtitlesegment.h` as the shared value object while library code owns collection-level behavior
