package aiplayer.ui.mobile.android.screens.player

sealed interface PlayerSideEffect {
    data object None : PlayerSideEffect
    data object TogglePlayback : PlayerSideEffect
    data class SeekRelative(val seconds: Int) : PlayerSideEffect
}
