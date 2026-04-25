package aiplayer.ui.mobile.android.model.ui

enum class BrowserBannerLevel {
    Info,
    Warning,
    Error,
}

data class BrowserBannerUiModel(
    val level: BrowserBannerLevel,
    val message: String,
    val actionLabel: String? = null,
)
