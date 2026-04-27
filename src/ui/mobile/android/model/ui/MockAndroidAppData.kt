package aiplayer.ui.mobile.android.model.ui

object MockAndroidAppData {
    fun playerSession(
        mediaTitle: String = "TED Talk - Learning Faster",
        mediaUri: String = "content://demo/video",
    ): PlayerSessionUiModel = PlayerSessionUiModel(
        mediaTitle = mediaTitle,
        mediaSourceLabel = mediaUri,
        currentTimeLabel = "00:12:20",
        durationLabel = "00:28:04",
        progressPercent = 44,
        isPlaying = true,
        isLoading = false,
        subtitleDisplayMode = SubtitleDisplayMode.Bilingual,
        translationStatus = TranslationUiStatus.Success,
        subtitleSummary = "英语原文 + 中文字幕",
    )
}
