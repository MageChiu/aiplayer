package aiplayer.platform.android.session

data class AndroidPlaybackSession(
    val mediaSource: AndroidMediaSource,
    val progress: AndroidPlaybackProgress = AndroidPlaybackProgress(),
    val subtitleSession: AndroidSubtitleSession = AndroidSubtitleSession(),
    val subtitlePreferences: AndroidSubtitlePreferences = AndroidSubtitlePreferences(),
    val translationPreferences: AndroidTranslationPreferences = AndroidTranslationPreferences(),
    val isPlaying: Boolean = false,
    val isLoading: Boolean = false,
)
