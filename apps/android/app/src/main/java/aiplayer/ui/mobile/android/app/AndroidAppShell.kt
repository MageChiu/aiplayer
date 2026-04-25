package aiplayer.ui.mobile.android.app

import aiplayer.ui.mobile.android.navigation.AndroidNavigator
import aiplayer.ui.mobile.android.navigation.AppRoute
import aiplayer.ui.mobile.android.navigation.NavigationState

class AndroidAppShell(
    initialState: NavigationState = NavigationState(),
) {
    private val navigator = AndroidNavigator(initialState)

    fun navigationState(): NavigationState = navigator.state

    fun openBrowser() {
        navigator.resetTo(AppRoute.Browser)
    }

    fun openPlayer() {
        navigator.navigateTo(AppRoute.Player)
    }

    fun openSettings() {
        navigator.navigateTo(AppRoute.Settings)
    }

    fun back(): Boolean = navigator.goBack()
}
