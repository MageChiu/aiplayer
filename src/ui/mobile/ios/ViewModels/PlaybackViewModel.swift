import Foundation

@MainActor
final class PlaybackViewModel: ObservableObject {
    @Published var recentItems: [PlaybackItemViewData]
    @Published var statusBannerText: String?
    @Published var statusStyle: StatusBannerView.Style
    @Published var activeItem: PlaybackItemViewData?
    @Published var isPlaying: Bool
    @Published var progress: Double
    @Published var elapsedText: String
    @Published var durationText: String
    @Published var playbackSpeed: Double
    @Published var isMuted: Bool
    @Published var isBuffering: Bool
    @Published var subtitleOriginalText: String
    @Published var subtitleTranslatedText: String
    @Published var subtitleStyle: SubtitleStyleViewData
    @Published var translationOption: TranslationOptionViewData

    init(
        recentItems: [PlaybackItemViewData],
        statusBannerText: String? = nil,
        statusStyle: StatusBannerView.Style = .info,
        activeItem: PlaybackItemViewData? = nil,
        isPlaying: Bool = false,
        progress: Double = 0.0,
        elapsedText: String = "00:00",
        durationText: String = "00:00",
        playbackSpeed: Double = 1.0,
        isMuted: Bool = false,
        isBuffering: Bool = false,
        subtitleOriginalText: String = "",
        subtitleTranslatedText: String = "",
        subtitleStyle: SubtitleStyleViewData = .default,
        translationOption: TranslationOptionViewData = .default
    ) {
        self.recentItems = recentItems
        self.statusBannerText = statusBannerText
        self.statusStyle = statusStyle
        self.activeItem = activeItem
        self.isPlaying = isPlaying
        self.progress = progress
        self.elapsedText = elapsedText
        self.durationText = durationText
        self.playbackSpeed = playbackSpeed
        self.isMuted = isMuted
        self.isBuffering = isBuffering
        self.subtitleOriginalText = subtitleOriginalText
        self.subtitleTranslatedText = subtitleTranslatedText
        self.subtitleStyle = subtitleStyle
        self.translationOption = translationOption
    }

    var continueWatchingItem: PlaybackItemViewData? {
        recentItems.first(where: { $0.progress > 0.0 && $0.progress < 0.98 })
    }

    static func makeHomeMock() -> PlaybackViewModel {
        PlaybackViewModel(
            recentItems: [
                PlaybackItemViewData.sample(
                    title: "Planet Earth Episode 1",
                    detail: "1080p / 46 min",
                    progress: 0.38,
                    subtitleSummary: "已加载中英字幕",
                    translationSummary: "在线翻译已开启",
                    status: "继续播放"
                ),
                PlaybackItemViewData.sample(
                    title: "TED Talk - Learning Faster",
                    detail: "720p / 18 min",
                    progress: 0.0,
                    subtitleSummary: "外挂字幕已匹配",
                    translationSummary: "翻译未开启",
                    status: "可播放"
                )
            ],
            statusBannerText: "iOS UI 壳层已就绪，下一步可导入本地视频。"
        )
    }
}
