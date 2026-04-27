package aiplayer.ui.mobile.android.screens.player.sheets

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.AssistChip
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.unit.dp
import aiplayer.ui.mobile.android.model.ui.SubtitleDisplayMode

@Composable
fun SubtitleSheetContent(
    selectedMode: SubtitleDisplayMode,
    onModeSelected: (SubtitleDisplayMode) -> Unit,
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(20.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text("字幕设置", style = MaterialTheme.typography.titleLarge)
        Text("切换原文、双语或仅译文显示模式。", style = MaterialTheme.typography.bodyMedium)
        listOf(
            SubtitleDisplayMode.OriginalOnly to "仅原文",
            SubtitleDisplayMode.Bilingual to "双语",
            SubtitleDisplayMode.TranslatedOnly to "仅译文",
        ).forEach { (mode, label) ->
            AssistChip(
                onClick = { onModeSelected(mode) },
                label = {
                    val suffix = if (mode == selectedMode) "（当前）" else ""
                    Text(label + suffix)
                },
            )
        }
    }
}
