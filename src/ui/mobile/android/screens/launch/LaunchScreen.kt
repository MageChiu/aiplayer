package aiplayer.ui.mobile.android.screens.launch

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp

@Composable
fun LaunchScreenRoute(
    canResumeSession: Boolean,
    resumeTitle: String?,
    onReady: () -> Unit,
    onResumeSession: () -> Unit,
) {
    val viewModel = remember { LaunchViewModel() }

    LaunchedEffect(canResumeSession, resumeTitle) {
        viewModel.beginInitialization()
        if (canResumeSession && !resumeTitle.isNullOrBlank()) {
            viewModel.markSessionDetected(resumeTitle)
        } else {
            viewModel.finishInitialization()
        }
    }

    LaunchScreen(
        state = viewModel.state,
        onContinue = onReady,
        onResumeSession = onResumeSession,
    )
}

@Composable
fun LaunchScreen(
    state: LaunchUiState,
    onContinue: () -> Unit,
    onResumeSession: () -> Unit,
) {
    Surface {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(24.dp),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            Text("AI Player", style = MaterialTheme.typography.headlineMedium)
            Text(state.statusMessage, style = MaterialTheme.typography.bodyLarge)
            state.resumeTitle?.let { title ->
                Text(
                    text = "最近会话：$title",
                    style = MaterialTheme.typography.bodyMedium,
                    textAlign = TextAlign.Center,
                )
            }
            if (state.canResumeSession) {
                Button(onClick = onResumeSession) {
                    Text("继续上次播放")
                }
            }
            Button(onClick = onContinue) {
                Text("进入首页")
            }
        }
    }
}
