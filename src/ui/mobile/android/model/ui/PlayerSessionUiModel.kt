package aiplayer.ui.mobile.android.model.ui

import aiplayer.platform.android.core.CoreSubtitleDisplayMode
import aiplayer.platform.android.core.CoreTranslationStatus

typealias SubtitleDisplayMode = CoreSubtitleDisplayMode
typealias TranslationUiStatus = CoreTranslationStatus

val TranslationUiStatus.isEnabled: Boolean
    get() = this != TranslationUiStatus.Disabled

val TranslationUiStatus.displayLabel: String
    get() = when (this) {
        TranslationUiStatus.Disabled -> "翻译已关闭"
        TranslationUiStatus.Idle -> "在线翻译可用"
        TranslationUiStatus.Requesting -> "翻译请求中"
        TranslationUiStatus.Success -> "最近一次翻译成功"
        TranslationUiStatus.Failed -> "最近一次翻译失败"
        TranslationUiStatus.Offline -> "当前处于离线状态"
    }

data class PlayerSessionUiModel(
    val mediaTitle: String = "",
    val mediaSourceLabel: String = "",
    val currentTimeLabel: String = "00:00",
    val durationLabel: String = "00:00",
    val progressPercent: Int = 0,
    val isPlaying: Boolean = false,
    val isLoading: Boolean = false,
    val subtitleDisplayMode: SubtitleDisplayMode = SubtitleDisplayMode.Bilingual,
    val translationStatus: TranslationUiStatus = TranslationUiStatus.Disabled,
    val subtitleSummary: String = "未加载字幕",
)
