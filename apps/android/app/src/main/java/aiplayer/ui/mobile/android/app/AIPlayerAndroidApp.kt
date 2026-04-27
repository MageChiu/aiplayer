package aiplayer.ui.mobile.android.app

import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.platform.LocalContext
import aiplayer.ui.mobile.android.data.local.AndroidSettingsStore
import aiplayer.ui.mobile.android.data.local.RecentPlaybackStore
import aiplayer.ui.mobile.android.navigation.AppRoute
import aiplayer.platform.android.player.PreviewAndroidPlayerController
import aiplayer.ui.mobile.android.screens.browser.BrowserScreenRoute
import aiplayer.ui.mobile.android.screens.launch.LaunchScreenRoute
import aiplayer.ui.mobile.android.screens.player.PlayerScreenRoute
import aiplayer.ui.mobile.android.screens.settings.SettingsScreenRoute
import aiplayer.platform.android.session.AndroidPlaybackSessionCoordinator
import aiplayer.platform.android.session.SharedPreferencesPlaybackSessionStore
import aiplayer.ui.mobile.android.theme.AIPlayerTheme

@Composable
fun AIPlayerAndroidApp() {
    AIPlayerTheme {
        val context = LocalContext.current
        val recentPlaybackStore = remember(context) { RecentPlaybackStore(context) }
        val settingsStore = remember(context) { AndroidSettingsStore(context) }
        val playbackSessionStore = remember(context) { SharedPreferencesPlaybackSessionStore(context) }
        val sessionCoordinator = remember(playbackSessionStore) {
            AndroidPlaybackSessionCoordinator(
                playerController = PreviewAndroidPlayerController(),
                sessionStore = playbackSessionStore,
            )
        }

        val currentRouteState = remember { mutableStateOf<AppRoute>(AppRoute.Launch) }
        val latestSession = remember(currentRouteState.value) { sessionCoordinator.currentSession() ?: playbackSessionStore.loadCurrentSession() }

        when (currentRouteState.value) {
            AppRoute.Launch -> LaunchScreenRoute(
                canResumeSession = latestSession != null,
                resumeTitle = latestSession?.mediaSource?.title,
                onReady = { currentRouteState.value = AppRoute.Browser },
                onResumeSession = {
                    val resumed = sessionCoordinator.resumeAvailableSession()
                    if (resumed != null) {
                        currentRouteState.value = AppRoute.Player
                    } else {
                        currentRouteState.value = AppRoute.Browser
                    }
                },
            )
            AppRoute.Browser -> BrowserScreenRoute(
                onOpenSettings = { currentRouteState.value = AppRoute.Settings },
                recentPlaybackStore = recentPlaybackStore,
                onOpenPlayer = { mediaTitle, mediaUri ->
                    sessionCoordinator.openMedia(title = mediaTitle, uri = mediaUri)
                    currentRouteState.value = AppRoute.Player
                },
            )
            AppRoute.Player -> PlayerScreenRoute(
                onBack = { currentRouteState.value = AppRoute.Browser },
                sessionCoordinator = sessionCoordinator,
            )
            AppRoute.Settings -> SettingsScreenRoute(
                onBack = { currentRouteState.value = AppRoute.Browser },
                settingsStore = settingsStore,
            )
        }
    }
}
