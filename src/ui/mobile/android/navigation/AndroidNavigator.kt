package aiplayer.ui.mobile.android.navigation

class AndroidNavigator(initialState: NavigationState = NavigationState()) {
    var state: NavigationState = initialState
        private set

    fun navigateTo(route: AppRoute) {
        if (state.currentRoute == route) {
            return
        }
        state = state.copy(
            currentRoute = route,
            backStack = state.backStack + route,
        )
    }

    fun goBack(): Boolean {
        if (state.backStack.size <= 1) {
            return false
        }
        val newStack = state.backStack.dropLast(1)
        state = state.copy(
            currentRoute = newStack.last(),
            backStack = newStack,
        )
        return true
    }

    fun resetTo(route: AppRoute) {
        state = NavigationState(
            currentRoute = route,
            backStack = listOf(route),
        )
    }
}
