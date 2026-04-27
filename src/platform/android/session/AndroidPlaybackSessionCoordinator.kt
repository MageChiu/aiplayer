package aiplayer.platform.android.session

import aiplayer.ui.mobile.android.model.ui.PlayerSessionUiModel
import aiplayer.ui.mobile.android.model.ui.SubtitleDisplayMode
import aiplayer.ui.mobile.android.model.ui.TranslationUiStatus
import aiplayer.platform.android.player.AndroidPlayerController

class AndroidPlaybackSessionCoordinator(
    private val playerController: AndroidPlayerController,
    private val sessionStore: AndroidPlaybackSessionStore,
) {
    private var currentSession: AndroidPlaybackSession? = sessionStore.loadCurrentSession()

    fun resumeAvailableSession(): AndroidPlaybackSession? {
        val storedSession = sessionStore.loadCurrentSession() ?: return null
        currentSession = storedSession
        playerController.open(storedSession.mediaSource)
        playerController.applySession(storedSession)
        syncFromPlayerSnapshot()
        return currentSession
    }

    fun openMedia(title: String, uri: String): AndroidPlaybackSession {
        val session = AndroidPlaybackSession(
            mediaSource = AndroidMediaSource(title = title, uri = uri),
        )
        currentSession = session
        playerController.open(session.mediaSource)
        playerController.applySession(session)
        syncFromPlayerSnapshot()
        return requireNotNull(currentSession)
    }

    fun attachSubtitle(label: String, subtitleUri: String): AndroidPlaybackSession? {
        val session = currentSession ?: return null
        currentSession = session.copy(
            subtitleSession = session.subtitleSession.copy(
                sourceUri = subtitleUri,
                label = label,
                attached = true,
            ),
        )
        persistCurrentSession()
        return currentSession
    }

    fun togglePlayback(): AndroidPlaybackSession? {
        if (currentSession == null) return null
        playerController.togglePlayback()
        syncFromPlayerSnapshot()
        return currentSession
    }

    fun seekRelative(seconds: Int): AndroidPlaybackSession? {
        if (currentSession == null) return null
        playerController.seekRelative(seconds)
        syncFromPlayerSnapshot()
        return currentSession
    }

    fun updateSubtitleMode(mode: SubtitleDisplayMode): AndroidPlaybackSession? {
        val session = currentSession ?: return null
        currentSession = session.copy(
            subtitlePreferences = session.subtitlePreferences.copy(mode = mode),
        )
        playerController.applySession(requireNotNull(currentSession))
        persistCurrentSession()
        return currentSession
    }

    fun updateTranslationEnabled(enabled: Boolean): AndroidPlaybackSession? {
        val session = currentSession ?: return null
        currentSession = session.copy(
            translationPreferences = session.translationPreferences.copy(enabled = enabled),
        )
        playerController.applySession(requireNotNull(currentSession))
        persistCurrentSession()
        return currentSession
    }

    fun updateTargetLanguage(language: String): AndroidPlaybackSession? {
        val session = currentSession ?: return null
        currentSession = session.copy(
            translationPreferences = session.translationPreferences.copy(targetLanguage = language),
        )
        playerController.applySession(requireNotNull(currentSession))
        persistCurrentSession()
        return currentSession
    }

    fun updateSubtitleFontScale(scale: Float): AndroidPlaybackSession? {
        val session = currentSession ?: return null
        currentSession = session.copy(
            subtitlePreferences = session.subtitlePreferences.copy(fontScale = scale),
        )
        playerController.applySession(requireNotNull(currentSession))
        persistCurrentSession()
        return currentSession
    }

    fun updateSubtitleBottomOffset(offsetPercent: Int): AndroidPlaybackSession? {
        val session = currentSession ?: return null
        currentSession = session.copy(
            subtitlePreferences = session.subtitlePreferences.copy(bottomOffsetPercent = offsetPercent),
        )
        playerController.applySession(requireNotNull(currentSession))
        persistCurrentSession()
        return currentSession
    }

    fun currentSession(): AndroidPlaybackSession? = currentSession

    fun currentUiSession(): PlayerSessionUiModel {
        val session = currentSession
        val snapshot = playerController.snapshot()
        if (session == null) {
            return PlayerSessionUiModel()
        }
        val translationStatus = if (session.translationPreferences.enabled) {
            TranslationUiStatus.Idle
        } else {
            TranslationUiStatus.Disabled
        }
        return PlayerSessionUiModel(
            mediaTitle = session.mediaSource.title,
            mediaSourceLabel = snapshot.mediaSourceLabel,
            currentTimeLabel = snapshot.currentTimeLabel,
            durationLabel = snapshot.durationLabel,
            progressPercent = snapshot.progressPercent,
            isPlaying = snapshot.isPlaying,
            isLoading = snapshot.isLoading,
            subtitleDisplayMode = session.subtitlePreferences.mode,
            translationStatus = translationStatus,
            subtitleSummary = buildSubtitleSummary(session, translationStatus),
        )
    }

    private fun syncFromPlayerSnapshot() {
        val session = currentSession ?: return
        val snapshot = playerController.snapshot()
        currentSession = session.copy(
            progress = AndroidPlaybackProgress(
                currentTimeLabel = snapshot.currentTimeLabel,
                durationLabel = snapshot.durationLabel,
                progressPercent = snapshot.progressPercent,
            ),
            isPlaying = snapshot.isPlaying,
            isLoading = snapshot.isLoading,
        )
        persistCurrentSession()
    }

    private fun persistCurrentSession() {
        currentSession?.let(sessionStore::saveCurrentSession)
    }

    private fun buildSubtitleSummary(
        session: AndroidPlaybackSession,
        translationStatus: TranslationUiStatus,
    ): String {
        val subtitleLabel = if (session.subtitleSession.attached) {
            session.subtitleSession.label
        } else {
            "未加载字幕"
        }
        val modeLabel = when (session.subtitlePreferences.mode) {
            SubtitleDisplayMode.OriginalOnly -> "仅原文"
            SubtitleDisplayMode.Bilingual -> "双语"
            SubtitleDisplayMode.TranslatedOnly -> "仅译文"
        }
        val translationLabel = if (translationStatus.isEnabled) {
            "在线翻译开启(${session.translationPreferences.targetLanguage})"
        } else {
            "翻译关闭"
        }
        return "$subtitleLabel | $modeLabel | $translationLabel | 字号 ${"%.1f".format(session.subtitlePreferences.fontScale)} | 偏移 ${session.subtitlePreferences.bottomOffsetPercent}%"
    }
}
