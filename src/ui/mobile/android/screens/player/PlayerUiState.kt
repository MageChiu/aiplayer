package aiplayer.ui.mobile.android.screens.player

import aiplayer.ui.mobile.android.model.ui.PlayerSessionUiModel
import aiplayer.ui.mobile.android.model.ui.SubtitleDisplayMode
import aiplayer.ui.mobile.android.model.ui.TranslationUiStatus

enum class PlayerSheet {
    None,
    Subtitle,
    Translation,
    SubtitleStyle,
    More,
}

data class PlayerUiState(
    val session: PlayerSessionUiModel = PlayerSessionUiModel(),
    val controlsVisible: Boolean = true,
    val activeSheet: PlayerSheet = PlayerSheet.None,
    val topStatusLabel: String = "未开始播放",
    val targetLanguage: String = "中文",
    val translationEnabled: Boolean = false,
    val subtitleFontScale: Float = 1.0f,
    val subtitleBottomOffsetPercent: Int = 12,
    val selectedSubtitleMode: SubtitleDisplayMode = SubtitleDisplayMode.Bilingual,
    val errorMessage: String? = null,
)
