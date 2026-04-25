package aiplayer.platform.android.session

interface AndroidPlaybackSessionStore {
    fun loadCurrentSession(): AndroidPlaybackSession?
    fun saveCurrentSession(session: AndroidPlaybackSession)
    fun clearCurrentSession()
}
