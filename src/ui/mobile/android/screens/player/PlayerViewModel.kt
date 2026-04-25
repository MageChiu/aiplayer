package aiplayer.ui.mobile.android.screens.player

import aiplayer.ui.mobile.android.model.ui.PlayerSessionUiModel
import aiplayer.ui.mobile.android.model.ui.SubtitleDisplayMode

class PlayerViewModel(
    initialState: PlayerUiState = PlayerUiState(),
) {
    var state: PlayerUiState = initialState
        private set

    fun bindSession(session: PlayerSessionUiModel) {
        state = state.copy(
            session = session,
            translationEnabled = session.translationStatus.isEnabled,
            selectedSubtitleMode = session.subtitleDisplayMode,
            topStatusLabel = when {
                session.isLoading -> "正在加载视频"
                session.isPlaying -> "播放中"
                else -> "已暂停"
            },
        )
    }

    fun updateTargetLanguage(language: String) {
        state = state.copy(targetLanguage = language)
    }

    fun updateTranslationEnabled(enabled: Boolean) {
        state = state.copy(translationEnabled = enabled)
    }

    fun updateSubtitleFontScale(scale: Float) {
        state = state.copy(subtitleFontScale = scale)
    }

    fun updateSubtitleBottomOffset(offsetPercent: Int) {
        state = state.copy(subtitleBottomOffsetPercent = offsetPercent)
    }

    fun updateSubtitleMode(mode: SubtitleDisplayMode) {
        state = state.copy(selectedSubtitleMode = mode)
    }

    fun showError(message: String) {
        state = state.copy(errorMessage = message)
    }

    fun clearError() {
        state = state.copy(errorMessage = null)
    }

    fun consume(intent: PlayerIntent): PlayerSideEffect {
        return when (intent) {
            PlayerIntent.TogglePlayback -> PlayerSideEffect.TogglePlayback
            PlayerIntent.SeekForwardQuickly -> PlayerSideEffect.SeekRelative(seconds = 10)
            PlayerIntent.SeekBackwardQuickly -> PlayerSideEffect.SeekRelative(seconds = -10)
            PlayerIntent.ToggleControlsVisibility -> {
                state = state.copy(controlsVisible = !state.controlsVisible)
                PlayerSideEffect.None
            }
            PlayerIntent.OpenSubtitleSheet -> {
                state = state.copy(activeSheet = PlayerSheet.Subtitle)
                PlayerSideEffect.None
            }
            PlayerIntent.OpenTranslationSheet -> {
                state = state.copy(activeSheet = PlayerSheet.Translation)
                PlayerSideEffect.None
            }
            PlayerIntent.OpenStyleSheet -> {
                state = state.copy(activeSheet = PlayerSheet.SubtitleStyle)
                PlayerSideEffect.None
            }
            PlayerIntent.OpenMoreSheet -> {
                state = state.copy(activeSheet = PlayerSheet.More)
                PlayerSideEffect.None
            }
            PlayerIntent.CloseSheets -> {
                state = state.copy(activeSheet = PlayerSheet.None)
                PlayerSideEffect.None
            }
        }
    }
}
