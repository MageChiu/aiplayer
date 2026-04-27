package aiplayer.ui.mobile.android.components.browser

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.AssistChip
import androidx.compose.material3.Card
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.unit.dp
import aiplayer.ui.mobile.android.model.ui.RecentMediaUiModel

@Composable
fun RecentMediaCard(
    item: RecentMediaUiModel,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
) {
    Card(onClick = onClick, modifier = modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
        ) {
            Text(item.title, style = MaterialTheme.typography.titleMedium)
            Text(item.subtitleLabel, style = MaterialTheme.typography.bodyMedium)
            Text(item.lastPositionLabel, style = MaterialTheme.typography.bodySmall)
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                if (item.hasSubtitle) {
                    AssistChip(onClick = {}, label = { Text("字幕") })
                }
                if (item.translationEnabled) {
                    AssistChip(onClick = {}, label = { Text("翻译") })
                }
                if (!item.isPlayable) {
                    AssistChip(onClick = {}, label = { Text("需重新授权") })
                }
            }
            Text(item.lastOpenedLabel, style = MaterialTheme.typography.labelSmall)
        }
    }
}
