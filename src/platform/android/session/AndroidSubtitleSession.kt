package aiplayer.platform.android.session

data class AndroidSubtitleSession(
    val sourceUri: String? = null,
    val label: String = "未加载字幕",
    val attached: Boolean = false,
)
