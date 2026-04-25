package aiplayer.ui.mobile.android.model.ui

data class RecentMediaUiModel(
    val id: String,
    val title: String,
    val mediaUri: String,
    val subtitleLabel: String,
    val lastPositionLabel: String,
    val lastOpenedLabel: String,
    val hasSubtitle: Boolean,
    val translationEnabled: Boolean,
    val isPlayable: Boolean,
)
