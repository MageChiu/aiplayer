package aiplayer.ui.mobile.android.screens.player

sealed interface PlayerIntent {
    data object TogglePlayback : PlayerIntent
    data object SeekForwardQuickly : PlayerIntent
    data object SeekBackwardQuickly : PlayerIntent
    data object ToggleControlsVisibility : PlayerIntent
    data object OpenSubtitleSheet : PlayerIntent
    data object OpenTranslationSheet : PlayerIntent
    data object OpenStyleSheet : PlayerIntent
    data object OpenMoreSheet : PlayerIntent
    data object CloseSheets : PlayerIntent
}
