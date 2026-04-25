package aiplayer.ui.mobile.android.screens.settings

sealed interface SettingsIntent {
    data class UpdateTargetLanguage(val language: String) : SettingsIntent
    data class ToggleAutoTranslation(val enabled: Boolean) : SettingsIntent
    data class ToggleAutoResume(val enabled: Boolean) : SettingsIntent
    data class ToggleKeepScreenOn(val enabled: Boolean) : SettingsIntent
    data object ClearCache : SettingsIntent
}
