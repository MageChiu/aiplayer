package aiplayer.ui.mobile.android.screens.launch

class LaunchViewModel(
    initialState: LaunchUiState = LaunchUiState(),
) {
    var state: LaunchUiState = initialState
        private set

    fun beginInitialization() {
        state = state.copy(
            stage = LaunchStage.Initializing,
            statusMessage = "正在初始化移动端能力",
            errorMessage = null,
        )
    }

    fun markSessionDetected(title: String) {
        state = state.copy(
            stage = LaunchStage.RestoringSession,
            statusMessage = "已发现最近播放会话",
            canResumeSession = true,
            resumeTitle = title,
        )
    }

    fun finishInitialization() {
        state = state.copy(
            stage = LaunchStage.Ready,
            statusMessage = "初始化完成",
            canResumeSession = false,
        )
    }

    fun fail(message: String) {
        state = state.copy(
            stage = LaunchStage.Error,
            statusMessage = "启动失败",
            errorMessage = message,
        )
    }
}
