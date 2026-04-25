package aiplayer.platform.android.player

import aiplayer.platform.android.session.AndroidMediaSource
import aiplayer.platform.android.session.AndroidPlaybackSession

interface AndroidPlayerController {
    fun open(mediaSource: AndroidMediaSource)
    fun applySession(session: AndroidPlaybackSession)
    fun togglePlayback()
    fun seekRelative(seconds: Int)
    fun snapshot(): AndroidPlayerSnapshot
}
