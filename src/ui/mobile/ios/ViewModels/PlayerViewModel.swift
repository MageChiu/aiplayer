import AVFoundation
import Foundation

@MainActor
final class PlayerViewModel: ObservableObject, Identifiable {
    let id = UUID()
    let resource: ImportedResource
    let sourceURL: URL
    let player: AVPlayer

    @Published var isPlaying = false
    @Published var progress: Double
    @Published var elapsedText = "00:00"
    @Published var durationText = "--:--"
    @Published var playbackSpeed = 1.0
    @Published var isMuted = false
    @Published var isBuffering = false
    @Published var subtitleOriginalText: String
    @Published var subtitleTranslatedText: String
    @Published var subtitleStyle = SubtitleStyleViewData.default
    @Published var translationOption = TranslationOptionViewData.default
    @Published var coreStateSnapshot: CorePlayerStateSnapshot

    private var timeObserverToken: Any?

    init(resource: ImportedResource, sourceURL: URL, linkedSubtitle: ImportedResource?) {
        self.resource = resource
        self.sourceURL = sourceURL
        self.player = AVPlayer(url: sourceURL)
        self.progress = resource.playbackProgress
        self.subtitleOriginalText = linkedSubtitle == nil ? "未绑定字幕，当前展示为占位文本。" : "已绑定字幕：\(linkedSubtitle!.displayName)"
        self.subtitleTranslatedText = linkedSubtitle == nil ? "导入字幕后可展示原文与译文双层字幕。" : "后续将从字幕解析结果驱动这一层译文显示。"
        self.coreStateSnapshot = CorePlayerStateSnapshot(
            playback: CorePlaybackStateSnapshot(
                loadState: .ready,
                paused: false,
                positionSeconds: 0,
                durationSeconds: 0,
                volume: 100,
                muted: false,
                loadedSource: sourceURL.lastPathComponent
            ),
            subtitles: CoreSubtitleStateSnapshot(
                displayMode: .bilingual,
                currentText: linkedSubtitle == nil ? "" : linkedSubtitle!.displayName,
                currentTranslatedText: "",
                statusMessage: linkedSubtitle == nil ? "未绑定字幕" : "字幕已绑定"
            ),
            translation: CoreTranslationStateSnapshot(
                status: .idle,
                enabled: true,
                mode: "online",
                provider: "pending",
                targetLanguage: "简体中文",
                lastError: ""
            )
        )
        configurePlayer()
    }

    deinit {
        if let token = timeObserverToken {
            player.removeTimeObserver(token)
        }
    }

    func togglePlayPause() {
        if isPlaying {
            player.pause()
            isPlaying = false
        } else {
            player.playImmediately(atRate: Float(playbackSpeed))
            isPlaying = true
        }
        syncCoreState()
    }

    func seekForward() {
        seek(by: 10)
    }

    func seekBackward() {
        seek(by: -10)
    }

    func cycleSpeed() {
        let speeds = [1.0, 1.25, 1.5, 2.0]
        let currentIndex = speeds.firstIndex(of: playbackSpeed) ?? 0
        playbackSpeed = speeds[(currentIndex + 1) % speeds.count]
        if isPlaying {
            player.rate = Float(playbackSpeed)
        }
        syncCoreState()
    }

    func toggleMute() {
        isMuted.toggle()
        player.isMuted = isMuted
        syncCoreState()
    }

    func prepareForDismissal() {
        player.pause()
        isPlaying = false
        syncCoreState()
    }

    func captureProgress() -> Double {
        progress
    }

    private func configurePlayer() {
        isMuted = player.isMuted
        addTimeObserver()
        player.playImmediately(atRate: Float(playbackSpeed))
        isPlaying = true
    }

    private func seek(by deltaSeconds: Double) {
        let current = player.currentTime().seconds.isFinite ? player.currentTime().seconds : 0
        let target = max(current + deltaSeconds, 0)
        let targetTime = CMTime(seconds: target, preferredTimescale: 600)
        player.seek(to: targetTime)
        syncCoreState()
    }

    private func addTimeObserver() {
        let interval = CMTime(seconds: 0.5, preferredTimescale: 600)
        timeObserverToken = player.addPeriodicTimeObserver(forInterval: interval, queue: .main) { [weak self] time in
            guard let self else { return }
            Task { @MainActor in
                let elapsed = time.seconds.isFinite ? time.seconds : 0
                let duration = self.player.currentItem?.duration.seconds ?? 0
                self.elapsedText = Self.timeString(from: elapsed)
                self.durationText = duration > 0 ? Self.timeString(from: duration) : "--:--"
                if duration > 0 {
                    self.progress = min(max(elapsed / duration, 0), 1)
                }
                self.isBuffering = self.player.timeControlStatus == .waitingToPlayAtSpecifiedRate
                self.syncCoreState(elapsed: elapsed, duration: max(duration, 0))
            }
        }
    }

    private func syncCoreState(elapsed: Double? = nil, duration: Double? = nil) {
        coreStateSnapshot = CorePlayerStateSnapshot(
            playback: CorePlaybackStateSnapshot(
                loadState: isBuffering ? .loading : .ready,
                paused: !isPlaying,
                positionSeconds: elapsed ?? player.currentTime().seconds,
                durationSeconds: duration ?? (player.currentItem?.duration.seconds ?? 0),
                volume: Int(player.volume * 100),
                muted: isMuted,
                loadedSource: sourceURL.lastPathComponent
            ),
            subtitles: CoreSubtitleStateSnapshot(
                displayMode: subtitleStyle.isVisible ? .bilingual : .originalOnly,
                currentText: subtitleOriginalText,
                currentTranslatedText: subtitleTranslatedText,
                statusMessage: subtitleStyle.isVisible ? "字幕显示中" : "字幕已隐藏"
            ),
            translation: CoreTranslationStateSnapshot(
                status: CoreTranslationStatusSnapshot(rawValue: translationOption.status.rawValue) ?? .idle,
                enabled: translationOption.isEnabled,
                mode: translationOption.displayMode,
                provider: "online",
                targetLanguage: translationOption.targetLanguage,
                lastError: translationOption.lastError
            )
        )
    }

    nonisolated private static func timeString(from seconds: Double) -> String {
        guard seconds.isFinite else { return "00:00" }
        let total = Int(seconds.rounded(.down))
        let hours = total / 3600
        let minutes = (total % 3600) / 60
        let secs = total % 60
        if hours > 0 {
            return String(format: "%d:%02d:%02d", hours, minutes, secs)
        }
        return String(format: "%02d:%02d", minutes, secs)
    }
}
