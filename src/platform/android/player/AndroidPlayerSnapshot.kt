package aiplayer.platform.android.player

data class AndroidPlayerSnapshot(
    val mediaSourceLabel: String,
    val currentTimeLabel: String,
    val durationLabel: String,
    val progressPercent: Int,
    val isPlaying: Boolean,
    val isLoading: Boolean,
)
