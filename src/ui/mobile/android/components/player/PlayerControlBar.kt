package aiplayer.ui.mobile.android.components.player

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import aiplayer.ui.mobile.android.model.ui.PlayerSessionUiModel

@Composable
fun PlayerControlBar(
    session: PlayerSessionUiModel,
    onTogglePlayback: () -> Unit,
    onSeekBackward: () -> Unit,
    onSeekForward: () -> Unit,
    modifier: Modifier = Modifier,
) {
    Column(modifier = modifier.fillMaxWidth(), verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceBetween) {
            Text(session.currentTimeLabel, style = MaterialTheme.typography.labelMedium)
            Text(session.durationLabel, style = MaterialTheme.typography.labelMedium)
        }
        Slider(value = session.progressPercent / 100f, onValueChange = {})
        Row(modifier = Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.SpaceEvenly) {
            OutlinedButton(onClick = onSeekBackward) { Text("-10s") }
            Button(onClick = onTogglePlayback) { Text(if (session.isPlaying) "暂停" else "播放") }
            OutlinedButton(onClick = onSeekForward) { Text("+10s") }
        }
    }
}
