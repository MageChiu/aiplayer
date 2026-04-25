package aiplayer.ui.mobile.android.screens.browser

import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import aiplayer.ui.mobile.android.components.browser.RecentMediaCard
import aiplayer.ui.mobile.android.components.common.EmptyStateCard
import aiplayer.ui.mobile.android.components.common.PrimaryActionCard
import aiplayer.ui.mobile.android.components.common.StatusBanner
import aiplayer.ui.mobile.android.data.local.RecentPlaybackStore

@Composable
fun BrowserScreenRoute(
    onOpenSettings: () -> Unit,
    onOpenPlayer: (String, String) -> Unit,
    recentPlaybackStore: RecentPlaybackStore,
) {
    val viewModel = remember { BrowserViewModel() }
    val openVideoPicker = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent(),
    ) { uri ->
        if (uri != null) {
            val mediaTitle = uri.lastPathSegment?.substringAfterLast('/')?.ifBlank { null }
                ?: "已选择本地视频"
            val mediaUri = uri.toString()
            recentPlaybackStore.saveOpenedMedia(mediaUri = mediaUri, title = mediaTitle)
            viewModel.loadRecentItems(recentPlaybackStore.loadRecentItems())
            onOpenPlayer(mediaTitle, mediaUri)
        }
    }
    val openSubtitlePicker = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent(),
    ) { uri ->
        if (uri != null) {
            val subtitleName = uri.lastPathSegment?.substringAfterLast('/')?.ifBlank { null }
                ?: "已选择字幕文件"
            val currentMedia = recentPlaybackStore.latest()
            if (currentMedia != null) {
                recentPlaybackStore.updateSubtitleState(
                    mediaUri = currentMedia.mediaUri,
                    subtitleLabel = subtitleName,
                    hasSubtitle = true,
                )
                viewModel.loadRecentItems(recentPlaybackStore.loadRecentItems())
                viewModel.showInfo("已为最近播放会话关联字幕：$subtitleName")
            } else {
                viewModel.showInfo("已选择字幕文件：$subtitleName。请先打开视频，后续将把字幕绑定到当前会话。")
            }
        }
    }

    LaunchedEffect(recentPlaybackStore) {
        viewModel.startLoading()
        viewModel.loadRecentItems(recentPlaybackStore.loadRecentItems())
    }

    BrowserScreen(
        state = viewModel.state,
        onOpenSettings = onOpenSettings,
        onPrimaryAction = {
            when (viewModel.consume(BrowserIntent.OpenVideoPicker)) {
                BrowserIntentResult.OpenVideoPicker -> openVideoPicker.launch("video/*")
                else -> Unit
            }
        },
        onSecondaryAction = {
            when (viewModel.consume(BrowserIntent.OpenSubtitlePicker)) {
                BrowserIntentResult.OpenSubtitlePicker -> openSubtitlePicker.launch("*/*")
                else -> Unit
            }
        },
        onRecentItemClick = { itemId ->
            val result = viewModel.consume(BrowserIntent.ResumeRecentMedia(itemId))
            if (result is BrowserIntentResult.ResumeRecentMedia) {
                val media = viewModel.state.recentItems.firstOrNull { it.id == itemId }
                if (media != null) {
                    onOpenPlayer(media.title, media.mediaUri)
                }
            }
        },
        onBannerAction = {
            viewModel.consume(BrowserIntent.DismissBanner)
        },
    )
}

@Composable
fun BrowserScreen(
    state: BrowserUiState,
    onOpenSettings: () -> Unit,
    onPrimaryAction: () -> Unit,
    onSecondaryAction: () -> Unit,
    onRecentItemClick: (String) -> Unit,
    onBannerAction: () -> Unit,
) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(state.title) },
                actions = {
                    TextButton(onClick = onOpenSettings) {
                        Text("设置")
                    }
                },
            )
        },
    ) { paddingValues ->
        LazyColumn(
            modifier = Modifier.fillMaxSize(),
            contentPadding = PaddingValues(
                start = 16.dp,
                end = 16.dp,
                top = paddingValues.calculateTopPadding() + 16.dp,
                bottom = 24.dp,
            ),
            verticalArrangement = Arrangement.spacedBy(16.dp),
        ) {
            item {
                PrimaryActionCard(
                    title = "开始一次新的观看会话",
                    subtitle = "先打开本地视频，再逐步加载字幕与在线翻译",
                    primaryLabel = state.primaryActionLabel,
                    secondaryLabel = state.secondaryActionLabel,
                    onPrimaryClick = onPrimaryAction,
                    onSecondaryClick = onSecondaryAction,
                )
            }
            state.banner?.let { banner ->
                item {
                    StatusBanner(
                        banner = banner,
                        onActionClick = onBannerAction,
                    )
                }
            }
            item {
                Text(
                    text = "最近播放",
                    style = MaterialTheme.typography.titleMedium,
                    modifier = Modifier.fillMaxWidth(),
                )
            }
            if (state.recentItems.isEmpty()) {
                item {
                    EmptyStateCard(
                        title = "暂无最近播放",
                        message = state.emptyMessage,
                    )
                }
            } else {
                items(state.recentItems, key = { it.id }) { item ->
                    RecentMediaCard(item = item, onClick = { onRecentItemClick(item.id) })
                }
            }
        }
    }
}
