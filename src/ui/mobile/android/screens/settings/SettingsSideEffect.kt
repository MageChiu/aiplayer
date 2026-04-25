package aiplayer.ui.mobile.android.screens.settings

sealed interface SettingsSideEffect {
    data object None : SettingsSideEffect
    data object ClearCache : SettingsSideEffect
}
