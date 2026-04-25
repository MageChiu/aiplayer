import SwiftUI

enum ColorTokens {
    static let accent = Color.blue
    static let playerBackground = Color.black
    static let subtitleBackground = Color.black.opacity(0.55)
    static let originalSubtitle = Color.white
    static let translatedSubtitle = Color(red: 0.72, green: 0.86, blue: 1.0)
    static let success = Color.green
    static let warning = Color.orange
    static let error = Color.red

    #if canImport(UIKit)
    static let surface = Color(UIColor.secondarySystemBackground)
    static let elevatedSurface = Color(UIColor.tertiarySystemBackground)
    #elseif canImport(AppKit)
    static let surface = Color(nsColor: .windowBackgroundColor)
    static let elevatedSurface = Color(nsColor: .underPageBackgroundColor)
    #else
    static let surface = Color.gray.opacity(0.12)
    static let elevatedSurface = Color.gray.opacity(0.18)
    #endif
}
