package aiplayer.platform.android.session

import android.content.Context
import aiplayer.ui.mobile.android.model.ui.SubtitleDisplayMode
import org.json.JSONObject

class SharedPreferencesPlaybackSessionStore(context: Context) : AndroidPlaybackSessionStore {
    private val preferences = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    override fun loadCurrentSession(): AndroidPlaybackSession? {
        val raw = preferences.getString(KEY_CURRENT_SESSION, null) ?: return null
        return runCatching {
            val json = JSONObject(raw)
            AndroidPlaybackSession(
                mediaSource = AndroidMediaSource(
                    title = json.optString("mediaTitle"),
                    uri = json.optString("mediaUri"),
                ),
                progress = AndroidPlaybackProgress(
                    currentTimeLabel = json.optString("currentTimeLabel", "00:00"),
                    durationLabel = json.optString("durationLabel", "00:00"),
                    progressPercent = json.optInt("progressPercent", 0),
                ),
                subtitleSession = AndroidSubtitleSession(
                    sourceUri = json.optString("subtitleUri").takeIf { it.isNotBlank() },
                    label = json.optString("subtitleLabel", "未加载字幕"),
                    attached = json.optBoolean("subtitleAttached", false),
                ),
                subtitlePreferences = AndroidSubtitlePreferences(
                    mode = parseSubtitleMode(json.optString("subtitleMode", SubtitleDisplayMode.Bilingual.name)),
                    fontScale = json.optDouble("subtitleFontScale", 1.0).toFloat(),
                    bottomOffsetPercent = json.optInt("subtitleBottomOffsetPercent", 12),
                ),
                translationPreferences = AndroidTranslationPreferences(
                    enabled = json.optBoolean("translationEnabled", false),
                    targetLanguage = json.optString("targetLanguage", "中文"),
                ),
                isPlaying = json.optBoolean("isPlaying", false),
                isLoading = json.optBoolean("isLoading", false),
            )
        }.getOrNull()
    }

    override fun saveCurrentSession(session: AndroidPlaybackSession) {
        val json = JSONObject()
            .put("mediaTitle", session.mediaSource.title)
            .put("mediaUri", session.mediaSource.uri)
            .put("currentTimeLabel", session.progress.currentTimeLabel)
            .put("durationLabel", session.progress.durationLabel)
            .put("progressPercent", session.progress.progressPercent)
            .put("subtitleUri", session.subtitleSession.sourceUri ?: "")
            .put("subtitleLabel", session.subtitleSession.label)
            .put("subtitleAttached", session.subtitleSession.attached)
            .put("subtitleMode", session.subtitlePreferences.mode.name)
            .put("subtitleFontScale", session.subtitlePreferences.fontScale)
            .put("subtitleBottomOffsetPercent", session.subtitlePreferences.bottomOffsetPercent)
            .put("translationEnabled", session.translationPreferences.enabled)
            .put("targetLanguage", session.translationPreferences.targetLanguage)
            .put("isPlaying", session.isPlaying)
            .put("isLoading", session.isLoading)
        preferences.edit().putString(KEY_CURRENT_SESSION, json.toString()).apply()
    }

    override fun clearCurrentSession() {
        preferences.edit().remove(KEY_CURRENT_SESSION).apply()
    }

    private fun parseSubtitleMode(value: String): SubtitleDisplayMode {
        return SubtitleDisplayMode.entries.firstOrNull { it.name == value } ?: SubtitleDisplayMode.Bilingual
    }

    companion object {
        private const val PREFS_NAME = "android_playback_session_store"
        private const val KEY_CURRENT_SESSION = "current_session"
    }
}
