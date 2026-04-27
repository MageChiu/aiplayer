package aiplayer.ui.mobile.android.data.local

import android.content.Context
import aiplayer.ui.mobile.android.screens.settings.SettingsUiState

class AndroidSettingsStore(context: Context) {
    private val preferences = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    fun load(): SettingsUiState {
        return SettingsUiState(
            defaultTargetLanguage = preferences.getString(KEY_TARGET_LANGUAGE, "中文") ?: "中文",
            autoEnableOnlineTranslation = preferences.getBoolean(KEY_AUTO_TRANSLATION, false),
            autoResumePlayback = preferences.getBoolean(KEY_AUTO_RESUME, true),
            keepScreenOn = preferences.getBoolean(KEY_KEEP_SCREEN_ON, true),
            defaultSubtitleModeLabel = preferences.getString(KEY_SUBTITLE_MODE, "双语") ?: "双语",
            cacheSizeLabel = preferences.getString(KEY_CACHE_SIZE, "0 MB") ?: "0 MB",
        )
    }

    fun persist(state: SettingsUiState) {
        preferences.edit()
            .putString(KEY_TARGET_LANGUAGE, state.defaultTargetLanguage)
            .putBoolean(KEY_AUTO_TRANSLATION, state.autoEnableOnlineTranslation)
            .putBoolean(KEY_AUTO_RESUME, state.autoResumePlayback)
            .putBoolean(KEY_KEEP_SCREEN_ON, state.keepScreenOn)
            .putString(KEY_SUBTITLE_MODE, state.defaultSubtitleModeLabel)
            .putString(KEY_CACHE_SIZE, state.cacheSizeLabel)
            .apply()
    }

    companion object {
        private const val PREFS_NAME = "android_settings_store"
        private const val KEY_TARGET_LANGUAGE = "target_language"
        private const val KEY_AUTO_TRANSLATION = "auto_translation"
        private const val KEY_AUTO_RESUME = "auto_resume"
        private const val KEY_KEEP_SCREEN_ON = "keep_screen_on"
        private const val KEY_SUBTITLE_MODE = "subtitle_mode"
        private const val KEY_CACHE_SIZE = "cache_size"
    }
}
