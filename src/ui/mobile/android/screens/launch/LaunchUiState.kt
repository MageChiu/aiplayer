package aiplayer.ui.mobile.android.screens.launch

import aiplayer.platform.android.core.CoreAppLaunchStage

typealias LaunchStage = CoreAppLaunchStage

data class LaunchUiState(
    val stage: LaunchStage = LaunchStage.Idle,
    val statusMessage: String = "准备启动",
    val canResumeSession: Boolean = false,
    val resumeTitle: String? = null,
    val errorMessage: String? = null,
)
