package aiplayer.ui.mobile.android.screens.settings

data class SettingsUiState(
    val defaultTargetLanguage: String = "中文",
    val autoEnableOnlineTranslation: Boolean = false,
    val autoResumePlayback: Boolean = true,
    val keepScreenOn: Boolean = true,
    val defaultSubtitleModeLabel: String = "双语",
    val cacheSizeLabel: String = "0 MB",
)
