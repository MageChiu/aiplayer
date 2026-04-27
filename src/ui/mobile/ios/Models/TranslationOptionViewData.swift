import Foundation

typealias TranslationUiStatus = CoreTranslationStatusSnapshot

extension CoreTranslationStatusSnapshot {
    var displayText: String {
        switch self {
        case .disabled:
            return "翻译已关闭"
        case .idle:
            return "在线翻译可用"
        case .requesting:
            return "翻译请求中"
        case .success:
            return "最近一次翻译成功"
        case .failed:
            return "最近一次翻译失败"
        case .offline:
            return "当前处于离线状态"
        }
    }
}

struct TranslationOptionViewData: Hashable {
    var status: TranslationUiStatus
    var targetLanguage: String
    var displayMode: String
    var lastError: String

    var isEnabled: Bool {
        get { status != .disabled }
        set {
            if newValue {
                status = status == .disabled ? .idle : status
            } else {
                status = .disabled
                lastError = ""
            }
        }
    }

    var statusText: String {
        if status == .failed && !lastError.isEmpty {
            return "翻译失败：\(lastError)"
        }
        return status.displayText
    }

    static let `default` = TranslationOptionViewData(
        status: .idle,
        targetLanguage: "简体中文",
        displayMode: "原文 + 译文",
        lastError: ""
    )
}
