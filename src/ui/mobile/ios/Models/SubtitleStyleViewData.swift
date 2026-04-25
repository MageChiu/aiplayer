import Foundation

struct SubtitleStyleViewData: Hashable {
    var isVisible: Bool
    var isBilingual: Bool
    var fontScale: Double
    var verticalOffset: Double

    static let `default` = SubtitleStyleViewData(
        isVisible: true,
        isBilingual: true,
        fontScale: 1.0,
        verticalOffset: 20.0
    )
}
