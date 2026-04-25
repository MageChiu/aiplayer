package aiplayer.ui.mobile.android.screens.player.sheets

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.AssistChip
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.unit.dp

@Composable
fun TranslationSheetContent(
    translationEnabled: Boolean,
    targetLanguage: String,
    onTranslationEnabledChange: (Boolean) -> Unit,
    onTargetLanguageSelected: (String) -> Unit,
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(20.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text("翻译设置", style = MaterialTheme.typography.titleLarge)
        Text("当前目标语言：$targetLanguage", style = MaterialTheme.typography.bodyMedium)
        Switch(
            checked = translationEnabled,
            onCheckedChange = onTranslationEnabledChange,
        )
        FlowRow(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            listOf("中文", "English", "日本語").forEach { language ->
                AssistChip(
                    onClick = { onTargetLanguageSelected(language) },
                    label = {
                        val suffix = if (language == targetLanguage) "（当前）" else ""
                        Text(language + suffix)
                    },
                )
            }
        }
    }
}
