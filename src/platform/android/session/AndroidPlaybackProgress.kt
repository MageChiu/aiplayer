package aiplayer.platform.android.session

data class AndroidPlaybackProgress(
    val currentTimeLabel: String = "00:00",
    val durationLabel: String = "00:00",
    val progressPercent: Int = 0,
)
