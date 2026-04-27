import Foundation

enum CoreAsyncLoadStateSnapshot: String, Hashable {
    case idle
    case loading
    case ready
    case empty
    case failed
}

enum CoreAppLaunchStageSnapshot: String, Hashable {
    case idle
    case initializing
    case restoringSession
    case ready
    case error
}

enum CoreAppRouteSnapshot: String, Hashable {
    case launch
    case browser
    case player
    case settings
}

enum CoreAppLifecycleStateSnapshot: String, Hashable {
    case inactive
    case foreground
    case background
}

enum CoreSubtitleDisplayModeSnapshot: String, Hashable {
    case originalOnly
    case bilingual
    case translatedOnly
}

enum CoreTranslationStatusSnapshot: String, Hashable {
    case disabled
    case idle
    case requesting
    case success
    case failed
    case offline
}

enum CoreMobileImportResourceKindSnapshot: String, Hashable {
    case video
    case subtitle
}

struct CoreNavigationStateSnapshot: Hashable {
    var currentRoute: CoreAppRouteSnapshot = .launch
    var canGoBack = false
}

struct CoreMobileShellStateSnapshot: Hashable {
    var launchStage: CoreAppLaunchStageSnapshot = .idle
    var lifecycle: CoreAppLifecycleStateSnapshot = .inactive
    var navigation = CoreNavigationStateSnapshot()
    var importState: CoreAsyncLoadStateSnapshot = .idle
}

struct CorePlaybackStateSnapshot: Hashable {
    var loadState: CoreAsyncLoadStateSnapshot = .idle
    var paused: Bool
    var positionSeconds: Double
    var durationSeconds: Double
    var volume: Int
    var muted: Bool
    var loadedSource: String
}

struct CoreSubtitleStateSnapshot: Hashable {
    var displayMode: CoreSubtitleDisplayModeSnapshot = .bilingual
    var currentText: String
    var currentTranslatedText: String
    var statusMessage: String
}

struct CoreTranslationStateSnapshot: Hashable {
    var status: CoreTranslationStatusSnapshot
    var enabled: Bool
    var mode: String
    var provider: String
    var targetLanguage: String
    var lastError: String
}

struct CorePlayerStateSnapshot: Hashable {
    var playback: CorePlaybackStateSnapshot
    var subtitles: CoreSubtitleStateSnapshot
    var translation: CoreTranslationStateSnapshot
}
