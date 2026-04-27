package aiplayer.ui.mobile.android.screens.player.sheets

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.unit.dp

@Composable
fun MoreSheetContent() {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(20.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp),
    ) {
        Text("更多操作", style = MaterialTheme.typography.titleLarge)
        Text("下一步将接入播放速度、重载视频、常亮与会话信息。", style = MaterialTheme.typography.bodyMedium)
    }
}
