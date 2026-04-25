package aiplayer.platform.android.player

import aiplayer.platform.android.session.AndroidMediaSource
import aiplayer.platform.android.session.AndroidPlaybackSession

class PreviewAndroidPlayerController : AndroidPlayerController {
    private var mediaSource: AndroidMediaSource = AndroidMediaSource(
        title = "TED Talk - Learning Faster",
        uri = "content://demo/video",
    )
    private var currentTimeLabel: String = "00:12:20"
    private var durationLabel: String = "00:28:04"
    private var progressPercent: Int = 44
    private var isPlaying: Boolean = true
    private var isLoading: Boolean = false

    override fun open(mediaSource: AndroidMediaSource) {
        this.mediaSource = mediaSource
        currentTimeLabel = "00:00"
        durationLabel = "00:28:04"
        progressPercent = 0
        isPlaying = false
        isLoading = false
    }

    override fun applySession(session: AndroidPlaybackSession) {
        mediaSource = session.mediaSource
        isPlaying = session.isPlaying
        isLoading = session.isLoading
    }

    override fun togglePlayback() {
        isPlaying = !isPlaying
    }

    override fun seekRelative(seconds: Int) {
        progressPercent = (progressPercent + seconds).coerceIn(0, 100)
    }

    override fun snapshot(): AndroidPlayerSnapshot {
        return AndroidPlayerSnapshot(
            mediaSourceLabel = mediaSource.uri,
            currentTimeLabel = currentTimeLabel,
            durationLabel = durationLabel,
            progressPercent = progressPercent,
            isPlaying = isPlaying,
            isLoading = isLoading,
        )
    }
}
