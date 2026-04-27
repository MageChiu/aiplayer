package aiplayer.ui.mobile.android.screens.browser

sealed interface BrowserIntentResult {
    data object None : BrowserIntentResult
    data object OpenVideoPicker : BrowserIntentResult
    data object OpenSubtitlePicker : BrowserIntentResult
    data object OpenSettings : BrowserIntentResult
    data object RetryPermission : BrowserIntentResult
    data class ResumeRecentMedia(val mediaId: String) : BrowserIntentResult
}
