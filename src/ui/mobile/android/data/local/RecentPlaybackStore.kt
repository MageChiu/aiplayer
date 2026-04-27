package aiplayer.ui.mobile.android.data.local

import android.content.Context
import aiplayer.ui.mobile.android.model.ui.RecentMediaUiModel
import org.json.JSONArray
import org.json.JSONObject
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

class RecentPlaybackStore(context: Context) {
    private val preferences = context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)
    private val formatter = DateTimeFormatter.ofPattern("MM-dd HH:mm")

    fun loadRecentItems(): List<RecentMediaUiModel> {
        val raw = preferences.getString(KEY_RECENT_ITEMS, null) ?: return emptyList()
        return runCatching {
            val jsonArray = JSONArray(raw)
            buildList {
                for (index in 0 until jsonArray.length()) {
                    val item = jsonArray.getJSONObject(index)
                    add(
                        RecentMediaUiModel(
                            id = item.optString("id"),
                            title = item.optString("title"),
                            mediaUri = item.optString("mediaUri"),
                            subtitleLabel = item.optString("subtitleLabel", "未加载字幕"),
                            lastPositionLabel = item.optString("lastPositionLabel", "00:00 / --:--"),
                            lastOpenedLabel = item.optString("lastOpenedLabel", "刚刚"),
                            hasSubtitle = item.optBoolean("hasSubtitle", false),
                            translationEnabled = item.optBoolean("translationEnabled", false),
                            isPlayable = item.optBoolean("isPlayable", true),
                        ),
                    )
                }
            }
        }.getOrDefault(emptyList())
    }

    fun latest(): RecentMediaUiModel? = loadRecentItems().firstOrNull()

    fun saveOpenedMedia(mediaUri: String, title: String) {
        val updatedItem = RecentMediaUiModel(
            id = mediaUri,
            title = title,
            mediaUri = mediaUri,
            subtitleLabel = "未加载字幕",
            lastPositionLabel = "00:00 / --:--",
            lastOpenedLabel = LocalDateTime.now().format(formatter),
            hasSubtitle = false,
            translationEnabled = false,
            isPlayable = true,
        )
        upsert(updatedItem)
    }

    fun updateSubtitleState(mediaUri: String, subtitleLabel: String, hasSubtitle: Boolean) {
        val items = loadRecentItems().toMutableList()
        val index = items.indexOfFirst { it.mediaUri == mediaUri }
        if (index >= 0) {
            items[index] = items[index].copy(
                subtitleLabel = subtitleLabel,
                hasSubtitle = hasSubtitle,
            )
            persist(items)
        }
    }

    private fun upsert(item: RecentMediaUiModel) {
        val items = loadRecentItems().toMutableList().apply {
            removeAll { it.mediaUri == item.mediaUri }
            add(0, item)
        }.take(MAX_RECENT_ITEMS)
        persist(items)
    }

    private fun persist(items: List<RecentMediaUiModel>) {
        val jsonArray = JSONArray()
        items.forEach { item ->
            jsonArray.put(
                JSONObject()
                    .put("id", item.id)
                    .put("title", item.title)
                    .put("mediaUri", item.mediaUri)
                    .put("subtitleLabel", item.subtitleLabel)
                    .put("lastPositionLabel", item.lastPositionLabel)
                    .put("lastOpenedLabel", item.lastOpenedLabel)
                    .put("hasSubtitle", item.hasSubtitle)
                    .put("translationEnabled", item.translationEnabled)
                    .put("isPlayable", item.isPlayable),
            )
        }
        preferences.edit().putString(KEY_RECENT_ITEMS, jsonArray.toString()).apply()
    }

    companion object {
        private const val PREFS_NAME = "android_recent_playback_store"
        private const val KEY_RECENT_ITEMS = "recent_items"
        private const val MAX_RECENT_ITEMS = 10
    }
}
