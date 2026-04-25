import Foundation

@MainActor
final class SettingsViewModel: ObservableObject {
    @Published var defaultPlaybackSpeed: Double
    @Published var autoResumeEnabled: Bool
    @Published var autoFullscreenOnRotate: Bool
    @Published var defaultBilingualSubtitle: Bool
    @Published var defaultTargetLanguage: String
    @Published var wifiOnlyTranslation: Bool
    @Published var diagnosticsEnabled: Bool

    init(
        defaultPlaybackSpeed: Double,
        autoResumeEnabled: Bool,
        autoFullscreenOnRotate: Bool,
        defaultBilingualSubtitle: Bool,
        defaultTargetLanguage: String,
        wifiOnlyTranslation: Bool,
        diagnosticsEnabled: Bool
    ) {
        self.defaultPlaybackSpeed = defaultPlaybackSpeed
        self.autoResumeEnabled = autoResumeEnabled
        self.autoFullscreenOnRotate = autoFullscreenOnRotate
        self.defaultBilingualSubtitle = defaultBilingualSubtitle
        self.defaultTargetLanguage = defaultTargetLanguage
        self.wifiOnlyTranslation = wifiOnlyTranslation
        self.diagnosticsEnabled = diagnosticsEnabled
    }

    static func makeMock() -> SettingsViewModel {
        SettingsViewModel(
            defaultPlaybackSpeed: 1.0,
            autoResumeEnabled: true,
            autoFullscreenOnRotate: true,
            defaultBilingualSubtitle: true,
            defaultTargetLanguage: "简体中文",
            wifiOnlyTranslation: true,
            diagnosticsEnabled: true
        )
    }
}
