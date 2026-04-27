package aiplayer.ui.mobile.android.screens.browser

import aiplayer.ui.mobile.android.model.ui.BrowserBannerUiModel
import aiplayer.ui.mobile.android.model.ui.RecentMediaUiModel

data class BrowserUiState(
    val isLoading: Boolean = false,
    val title: String = "AI Player",
    val primaryActionLabel: String = "打开视频",
    val secondaryActionLabel: String = "打开字幕",
    val recentItems: List<RecentMediaUiModel> = emptyList(),
    val banner: BrowserBannerUiModel? = null,
    val hasPermissionIssue: Boolean = false,
    val emptyMessage: String = "还没有最近播放记录",
)
