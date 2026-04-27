package aiplayer.ui.mobile.android.screens.settings

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.AssistChip
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.unit.dp
import aiplayer.ui.mobile.android.components.settings.SettingsGroupCard
import aiplayer.ui.mobile.android.data.local.AndroidSettingsStore

@Composable
fun SettingsScreenRoute(
    onBack: () -> Unit,
    settingsStore: AndroidSettingsStore,
) {
    val viewModel = remember { SettingsViewModel() }

    LaunchedEffect(settingsStore) {
        viewModel.loadState(settingsStore.load())
    }

    SettingsScreen(
        state = viewModel.state,
        onBack = onBack,
        onTargetLanguageSelected = { language ->
            viewModel.consume(SettingsIntent.UpdateTargetLanguage(language))
            settingsStore.persist(viewModel.state)
        },
        onToggleAutoTranslation = { enabled ->
            viewModel.consume(SettingsIntent.ToggleAutoTranslation(enabled))
            settingsStore.persist(viewModel.state)
        },
        onToggleAutoResume = { enabled ->
            viewModel.consume(SettingsIntent.ToggleAutoResume(enabled))
            settingsStore.persist(viewModel.state)
        },
        onToggleKeepScreenOn = { enabled ->
            viewModel.consume(SettingsIntent.ToggleKeepScreenOn(enabled))
            settingsStore.persist(viewModel.state)
        },
        onClearCache = {
            viewModel.consume(SettingsIntent.ClearCache)
            settingsStore.persist(viewModel.state)
        },
    )
}

@Composable
fun SettingsScreen(
    state: SettingsUiState,
    onBack: () -> Unit,
    onTargetLanguageSelected: (String) -> Unit,
    onToggleAutoTranslation: (Boolean) -> Unit,
    onToggleAutoResume: (Boolean) -> Unit,
    onToggleKeepScreenOn: (Boolean) -> Unit,
    onClearCache: () -> Unit,
) {
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("设置") },
                navigationIcon = {
                    TextButton(onClick = onBack) {
                        Text("返回")
                    }
                },
            )
        },
    ) { paddingValues ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .verticalScroll(rememberScrollState())
                .padding(
                    PaddingValues(
                        start = 16.dp,
                        end = 16.dp,
                        top = paddingValues.calculateTopPadding() + 16.dp,
                        bottom = 24.dp,
                    ),
                ),
            verticalArrangement = Arrangement.spacedBy(16.dp),
        ) {
            SettingsGroupCard(title = "翻译") {
                SettingLanguageSelector(
                    currentLanguage = state.defaultTargetLanguage,
                    onLanguageSelected = onTargetLanguageSelected,
                )
                HorizontalDivider()
                SettingToggleRow(
                    label = "自动开启在线翻译",
                    description = "有网络时自动进入双语模式",
                    checked = state.autoEnableOnlineTranslation,
                    onCheckedChange = onToggleAutoTranslation,
                )
            }
            SettingsGroupCard(title = "播放") {
                SettingToggleRow(
                    label = "自动恢复播放进度",
                    description = "重新打开最近媒体时恢复上次时间点",
                    checked = state.autoResumePlayback,
                    onCheckedChange = onToggleAutoResume,
                )
                HorizontalDivider()
                SettingToggleRow(
                    label = "保持屏幕常亮",
                    description = "在播放页面停留时保持唤醒",
                    checked = state.keepScreenOn,
                    onCheckedChange = onToggleKeepScreenOn,
                )
            }
            SettingsGroupCard(title = "缓存") {
                SettingTextRow(label = "缓存占用", value = state.cacheSizeLabel)
                HorizontalDivider()
                TextButton(onClick = onClearCache) {
                    Text("清理缓存")
                }
            }
        }
    }
}

@Composable
private fun SettingLanguageSelector(
    currentLanguage: String,
    onLanguageSelected: (String) -> Unit,
) {
    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(8.dp),
    ) {
        Text("默认目标语言", style = MaterialTheme.typography.titleSmall)
        Text("当前：$currentLanguage", style = MaterialTheme.typography.bodyMedium)
        Row(
            horizontalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            listOf("中文", "English", "日本語").forEach { language ->
                AssistChip(
                    onClick = { onLanguageSelected(language) },
                    label = { Text(language) },
                )
            }
        }
    }
}

@Composable
private fun SettingToggleRow(
    label: String,
    description: String,
    checked: Boolean,
    onCheckedChange: (Boolean) -> Unit,
) {
    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(8.dp),
    ) {
        Text(label, style = MaterialTheme.typography.titleSmall)
        Text(description, style = MaterialTheme.typography.bodyMedium)
        Switch(checked = checked, onCheckedChange = onCheckedChange)
    }
}

@Composable
private fun SettingTextRow(
    label: String,
    value: String,
) {
    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(8.dp),
    ) {
        Text(label, style = MaterialTheme.typography.titleSmall)
        Text(value, style = MaterialTheme.typography.bodyMedium)
    }
}
