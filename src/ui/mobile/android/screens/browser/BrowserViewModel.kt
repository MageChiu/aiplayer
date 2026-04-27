package aiplayer.ui.mobile.android.screens.browser

import aiplayer.ui.mobile.android.model.ui.BrowserBannerLevel
import aiplayer.ui.mobile.android.model.ui.BrowserBannerUiModel
import aiplayer.ui.mobile.android.model.ui.RecentMediaUiModel

class BrowserViewModel(
    initialState: BrowserUiState = BrowserUiState(),
) {
    var state: BrowserUiState = initialState
        private set

    fun loadRecentItems(items: List<RecentMediaUiModel>) {
        state = state.copy(
            isLoading = false,
            recentItems = items,
            banner = if (items.isEmpty()) {
                BrowserBannerUiModel(
                    level = BrowserBannerLevel.Info,
                    message = "先打开一个本地视频，后续可从这里继续播放",
                )
            } else {
                null
            },
        )
    }

    fun startLoading() {
        state = state.copy(isLoading = true)
    }

    fun showPermissionIssue(message: String) {
        state = state.copy(
            isLoading = false,
            hasPermissionIssue = true,
            banner = BrowserBannerUiModel(
                level = BrowserBannerLevel.Warning,
                message = message,
                actionLabel = "重新授权",
            ),
        )
    }

    fun showError(message: String) {
        state = state.copy(
            isLoading = false,
            banner = BrowserBannerUiModel(
                level = BrowserBannerLevel.Error,
                message = message,
                actionLabel = "重试",
            ),
        )
    }

    fun showInfo(message: String) {
        state = state.copy(
            isLoading = false,
            hasPermissionIssue = false,
            banner = BrowserBannerUiModel(
                level = BrowserBannerLevel.Info,
                message = message,
                actionLabel = "知道了",
            ),
        )
    }

    fun consume(intent: BrowserIntent): BrowserIntentResult {
        return when (intent) {
            BrowserIntent.OpenVideoPicker -> BrowserIntentResult.OpenVideoPicker
            BrowserIntent.OpenSubtitlePicker -> BrowserIntentResult.OpenSubtitlePicker
            BrowserIntent.OpenSettings -> BrowserIntentResult.OpenSettings
            is BrowserIntent.ResumeRecentMedia -> BrowserIntentResult.ResumeRecentMedia(intent.mediaId)
            BrowserIntent.RetryPermission -> BrowserIntentResult.RetryPermission
            BrowserIntent.DismissBanner -> {
                state = state.copy(banner = null)
                BrowserIntentResult.None
            }
        }
    }
}
