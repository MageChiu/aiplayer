package aiplayer.ui.mobile.android.screens.player.sheets

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.unit.dp

@Composable
fun SubtitleStyleSheetContent(
    fontScale: Float,
    bottomOffsetPercent: Int,
    onFontScaleChanged: (Float) -> Unit,
    onBottomOffsetChanged: (Int) -> Unit,
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(20.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text("字幕样式", style = MaterialTheme.typography.titleLarge)
        Text("字号：${"%.1f".format(fontScale)}", style = MaterialTheme.typography.bodyMedium)
        Slider(
            value = fontScale,
            onValueChange = onFontScaleChanged,
            valueRange = 0.8f..1.6f,
        )
        Text("底部偏移：$bottomOffsetPercent%", style = MaterialTheme.typography.bodyMedium)
        Slider(
            value = bottomOffsetPercent.toFloat(),
            onValueChange = { onBottomOffsetChanged(it.toInt()) },
            valueRange = 0f..30f,
        )
    }
}
