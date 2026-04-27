package aiplayer.ui.mobile.android.screens.browser

sealed interface BrowserIntent {
    data object OpenVideoPicker : BrowserIntent
    data object OpenSubtitlePicker : BrowserIntent
    data object OpenSettings : BrowserIntent
    data class ResumeRecentMedia(val mediaId: String) : BrowserIntent
    data object RetryPermission : BrowserIntent
    data object DismissBanner : BrowserIntent
}
