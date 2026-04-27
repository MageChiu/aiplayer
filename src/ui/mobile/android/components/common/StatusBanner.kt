package aiplayer.ui.mobile.android.components.common

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.weight
import androidx.compose.material3.AssistChip
import androidx.compose.material3.Card
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.unit.dp
import aiplayer.ui.mobile.android.model.ui.BrowserBannerUiModel

@Composable
fun StatusBanner(
    banner: BrowserBannerUiModel,
    onActionClick: (() -> Unit)? = null,
    modifier: Modifier = Modifier,
) {
    Card(modifier = modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier.padding(16.dp),
            horizontalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            Text(
                text = banner.message,
                style = MaterialTheme.typography.bodyMedium,
                modifier = Modifier.weight(1f),
            )
            if (banner.actionLabel != null && onActionClick != null) {
                AssistChip(
                    onClick = onActionClick,
                    label = { Text(banner.actionLabel) },
                )
            }
        }
    }
}
