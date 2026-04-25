package aiplayer.ui.mobile.android.screens.player

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.weight
import androidx.compose.material3.AssistChip
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import aiplayer.ui.mobile.android.components.player.PlayerControlBar
import aiplayer.ui.mobile.android.components.player.SubtitleOverlayCard
import aiplayer.ui.mobile.android.model.ui.SubtitleDisplayMode
import aiplayer.ui.mobile.android.screens.player.sheets.MoreSheetContent
import aiplayer.ui.mobile.android.screens.player.sheets.SubtitleSheetContent
import aiplayer.ui.mobile.android.screens.player.sheets.SubtitleStyleSheetContent
import aiplayer.ui.mobile.android.screens.player.sheets.TranslationSheetContent
import aiplayer.platform.android.session.AndroidPlaybackSessionCoordinator

@Composable
fun PlayerScreenRoute(
    onBack: () -> Unit,
    sessionCoordinator: AndroidPlaybackSessionCoordinator,
) {
    val viewModel = remember { PlayerViewModel() }

    LaunchedEffect(sessionCoordinator) {
        viewModel.bindSession(sessionCoordinator.currentUiSession())
    }

    PlayerScreen(
        state = viewModel.state,
        onBack = onBack,
        onTogglePlayback = {
            when (viewModel.consume(PlayerIntent.TogglePlayback)) {
                PlayerSideEffect.TogglePlayback -> {
                    sessionCoordinator.togglePlayback()
                    viewModel.bindSession(sessionCoordinator.currentUiSession())
                }
                else -> Unit
            }
        },
        onSeekBackward = {
            when (val effect = viewModel.consume(PlayerIntent.SeekBackwardQuickly)) {
                is PlayerSideEffect.SeekRelative -> {
                    sessionCoordinator.seekRelative(effect.seconds)
                    viewModel.bindSession(sessionCoordinator.currentUiSession())
                }
                else -> Unit
            }
        },
        onSeekForward = {
            when (val effect = viewModel.consume(PlayerIntent.SeekForwardQuickly)) {
                is PlayerSideEffect.SeekRelative -> {
                    sessionCoordinator.seekRelative(effect.seconds)
                    viewModel.bindSession(sessionCoordinator.currentUiSession())
                }
                else -> Unit
            }
        },
        onOpenSubtitleSheet = { viewModel.consume(PlayerIntent.OpenSubtitleSheet) },
        onOpenTranslationSheet = { viewModel.consume(PlayerIntent.OpenTranslationSheet) },
        onOpenStyleSheet = { viewModel.consume(PlayerIntent.OpenStyleSheet) },
        onOpenMoreSheet = { viewModel.consume(PlayerIntent.OpenMoreSheet) },
        onDismissSheet = { viewModel.consume(PlayerIntent.CloseSheets) },
        onSubtitleModeSelected = { mode ->
            viewModel.updateSubtitleMode(mode)
            sessionCoordinator.updateSubtitleMode(mode)
            viewModel.bindSession(sessionCoordinator.currentUiSession())
        },
        onTranslationEnabledChange = { enabled ->
            viewModel.updateTranslationEnabled(enabled)
            sessionCoordinator.updateTranslationEnabled(enabled)
            viewModel.bindSession(sessionCoordinator.currentUiSession())
        },
        onTargetLanguageSelected = { language ->
            viewModel.updateTargetLanguage(language)
            sessionCoordinator.updateTargetLanguage(language)
            viewModel.bindSession(sessionCoordinator.currentUiSession())
        },
        onFontScaleChanged = { scale ->
            viewModel.updateSubtitleFontScale(scale)
            sessionCoordinator.updateSubtitleFontScale(scale)
            viewModel.bindSession(sessionCoordinator.currentUiSession())
        },
        onBottomOffsetChanged = { offset ->
            viewModel.updateSubtitleBottomOffset(offset)
            sessionCoordinator.updateSubtitleBottomOffset(offset)
            viewModel.bindSession(sessionCoordinator.currentUiSession())
        },
    )
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun PlayerScreen(
    state: PlayerUiState,
    onBack: () -> Unit,
    onTogglePlayback: () -> Unit,
    onSeekBackward: () -> Unit,
    onSeekForward: () -> Unit,
    onOpenSubtitleSheet: () -> Unit,
    onOpenTranslationSheet: () -> Unit,
    onOpenStyleSheet: () -> Unit,
    onOpenMoreSheet: () -> Unit,
    onDismissSheet: () -> Unit,
    onSubtitleModeSelected: (SubtitleDisplayMode) -> Unit,
    onTranslationEnabledChange: (Boolean) -> Unit,
    onTargetLanguageSelected: (String) -> Unit,
    onFontScaleChanged: (Float) -> Unit,
    onBottomOffsetChanged: (Int) -> Unit,
) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Text(
                        text = state.session.mediaTitle.ifBlank { "播放器" },
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis,
                    )
                },
                navigationIcon = {
                    TextButton(onClick = onBack) {
                        Text("返回")
                    }
                },
            )
        },
    ) { paddingValues ->
        Surface(
            modifier = Modifier
                .fillMaxSize()
                .padding(paddingValues),
        ) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(16.dp),
            ) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f)
                        .background(Color.Black),
                    contentAlignment = Alignment.Center,
                ) {
                    Text(
                        text = if (state.session.isLoading) "播放器加载中" else "视频渲染区域占位",
                        color = Color.White,
                    )
                }
                if (state.session.mediaSourceLabel.isNotBlank()) {
                    Text(
                        text = state.session.mediaSourceLabel,
                        style = MaterialTheme.typography.labelSmall,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis,
                    )
                }
                SubtitleOverlayCard(
                    originalText = "The more you practice, the faster you improve.",
                    translatedText = "你练习得越多，进步就越快。",
                )
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    AssistChip(onClick = onOpenSubtitleSheet, label = { Text("字幕") })
                    AssistChip(onClick = onOpenTranslationSheet, label = { Text("翻译") })
                    AssistChip(onClick = onOpenStyleSheet, label = { Text("样式") })
                    AssistChip(onClick = onOpenMoreSheet, label = { Text("更多") })
                }
                Text(
                    text = state.session.subtitleSummary,
                    style = MaterialTheme.typography.bodyMedium,
                )
                PlayerControlBar(
                    session = state.session,
                    onTogglePlayback = onTogglePlayback,
                    onSeekBackward = onSeekBackward,
                    onSeekForward = onSeekForward,
                )
                state.errorMessage?.let { errorMessage ->
                    Text(
                        text = errorMessage,
                        color = MaterialTheme.colorScheme.error,
                        style = MaterialTheme.typography.bodyMedium,
                    )
                }
            }
        }
    }

    if (state.activeSheet != PlayerSheet.None) {
        ModalBottomSheet(onDismissRequest = onDismissSheet) {
            when (state.activeSheet) {
                PlayerSheet.Subtitle -> SubtitleSheetContent(
                    selectedMode = state.selectedSubtitleMode,
                    onModeSelected = onSubtitleModeSelected,
                )
                PlayerSheet.Translation -> TranslationSheetContent(
                    translationEnabled = state.translationEnabled,
                    targetLanguage = state.targetLanguage,
                    onTranslationEnabledChange = onTranslationEnabledChange,
                    onTargetLanguageSelected = onTargetLanguageSelected,
                )
                PlayerSheet.SubtitleStyle -> SubtitleStyleSheetContent(
                    fontScale = state.subtitleFontScale,
                    bottomOffsetPercent = state.subtitleBottomOffsetPercent,
                    onFontScaleChanged = onFontScaleChanged,
                    onBottomOffsetChanged = onBottomOffsetChanged,
                )
                PlayerSheet.More -> MoreSheetContent()
                PlayerSheet.None -> Unit
            }
        }
    }
}
