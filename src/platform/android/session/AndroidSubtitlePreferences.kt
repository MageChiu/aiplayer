package aiplayer.platform.android.session

import aiplayer.ui.mobile.android.model.ui.SubtitleDisplayMode

data class AndroidSubtitlePreferences(
    val mode: SubtitleDisplayMode = SubtitleDisplayMode.Bilingual,
    val fontScale: Float = 1.0f,
    val bottomOffsetPercent: Int = 12,
)
