package aiplayer.ui.mobile.android.screens.settings

class SettingsViewModel(
    initialState: SettingsUiState = SettingsUiState(),
) {
    var state: SettingsUiState = initialState
        private set

    fun loadState(loadedState: SettingsUiState) {
        state = loadedState
    }

    fun consume(intent: SettingsIntent): SettingsSideEffect {
        return when (intent) {
            is SettingsIntent.UpdateTargetLanguage -> {
                state = state.copy(defaultTargetLanguage = intent.language)
                SettingsSideEffect.None
            }
            is SettingsIntent.ToggleAutoTranslation -> {
                state = state.copy(autoEnableOnlineTranslation = intent.enabled)
                SettingsSideEffect.None
            }
            is SettingsIntent.ToggleAutoResume -> {
                state = state.copy(autoResumePlayback = intent.enabled)
                SettingsSideEffect.None
            }
            is SettingsIntent.ToggleKeepScreenOn -> {
                state = state.copy(keepScreenOn = intent.enabled)
                SettingsSideEffect.None
            }
            SettingsIntent.ClearCache -> {
                state = state.copy(cacheSizeLabel = "0 MB")
                SettingsSideEffect.ClearCache
            }
        }
    }
}
