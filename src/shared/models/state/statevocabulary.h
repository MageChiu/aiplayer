#pragma once

enum class AsyncLoadState {
    Idle,
    Loading,
    Ready,
    Empty,
    Failed,
};

enum class AppLaunchStage {
    Idle,
    Initializing,
    RestoringSession,
    Ready,
    Error,
};

enum class AppRouteId {
    Launch,
    Browser,
    Player,
    Settings,
};

enum class AppLifecycleState {
    Inactive,
    Foreground,
    Background,
};

enum class SubtitleDisplayMode {
    OriginalOnly,
    Bilingual,
    TranslatedOnly,
};

struct NavigationStateSnapshot {
    AppRouteId currentRoute = AppRouteId::Launch;
    bool canGoBack = false;
};
