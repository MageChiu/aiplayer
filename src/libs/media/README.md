# Media Library

`src/libs/media` is the planned home for reusable media discovery and preprocessing helpers that can serve desktop first without dragging player UI into the library boundary.

## Layout
- `include/`: future public headers for media discovery and preprocessing services
- `src/`: future implementation of file classification and FFmpeg-based preprocessing helpers
- `tests/`: focused checks for media-file discovery and preprocessing edge cases

## Candidate library logic
- Playable-file classification and directory scanning helpers currently embedded in `mpvwidget.cpp`, such as suffix filtering and largest-playable-file selection
- FFmpeg-based audio preprocessing helpers that decode media into WAV/PCM and do not require widget state, mpv handles, or UI callbacks
- Media metadata normalization or preprocessing result models once desktop and mobile need the same representation

## Deferred logic and reasons
- Torrent prebuffering stays outside because it couples libtorrent session lifecycle with product-level playback callbacks
- mpv player control, rendering, and seek/load commands stay outside because they are product/runtime integration rather than reusable media library logic
- Temp-file lifecycle, worker-thread ownership, and user-facing error presentation stay outside until a non-UI coordinator exists above the library

## Current migration strategy
- Step 1: extract pure media discovery helpers and FFmpeg preprocessing functions behind backend-neutral interfaces
- Step 2: let desktop product code keep thread scheduling, cancellation ownership, and playback wiring
- Step 3: revisit a shared media cache only after Android or iOS need the same preprocessing pipeline
