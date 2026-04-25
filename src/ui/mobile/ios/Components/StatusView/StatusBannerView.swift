import SwiftUI

struct StatusBannerView: View {
    enum Style {
        case info
        case success
        case warning
        case error

        var color: Color {
            switch self {
            case .info:
                return ColorTokens.accent
            case .success:
                return ColorTokens.success
            case .warning:
                return ColorTokens.warning
            case .error:
                return ColorTokens.error
            }
        }

        var icon: String {
            switch self {
            case .info:
                return "info.circle"
            case .success:
                return "checkmark.circle"
            case .warning:
                return "exclamationmark.triangle"
            case .error:
                return "xmark.octagon"
            }
        }
    }

    let text: String
    let style: Style

    var body: some View {
        HStack(alignment: .top, spacing: SpacingTokens.small) {
            Image(systemName: style.icon)
                .foregroundStyle(style.color)
            Text(text)
                .font(TypographyTokens.caption)
                .foregroundStyle(.primary)
            Spacer(minLength: 0)
        }
        .padding(SpacingTokens.medium)
        .background(ColorTokens.surface)
        .clipShape(RoundedRectangle(cornerRadius: 16, style: .continuous))
    }
}
