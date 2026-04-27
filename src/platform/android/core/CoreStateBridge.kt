package aiplayer.platform.android.core

enum class CoreAsyncLoadState {
    Idle,
    Loading,
    Ready,
    Empty,
    Failed,
}

enum class CoreAppLaunchStage {
    Idle,
    Initializing,
    RestoringSession,
    Ready,
    Error,
}

enum class CoreAppRoute(val routeName: String) {
    Launch("launch"),
    Browser("browser"),
    Player("player"),
    Settings("settings"),
}

enum class CoreAppLifecycleState {
    Inactive,
    Foreground,
    Background,
}

enum class CoreSubtitleDisplayMode {
    OriginalOnly,
    Bilingual,
    TranslatedOnly,
}

enum class CoreTranslationStatus {
    Disabled,
    Idle,
    Requesting,
    Success,
    Failed,
    Offline,
}

enum class CoreMobileImportResourceKind {
    Video,
    Subtitle,
}

data class CoreNavigationState(
    val currentRoute: CoreAppRoute = CoreAppRoute.Launch,
    val backStack: List<CoreAppRoute> = listOf(CoreAppRoute.Launch),
) {
    val canGoBack: Boolean
        get() = backStack.size > 1
}
