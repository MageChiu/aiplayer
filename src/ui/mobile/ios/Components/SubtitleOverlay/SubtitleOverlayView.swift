import SwiftUI

struct SubtitleOverlayView: View {
    let originalText: String
    let translatedText: String
    let style: SubtitleStyleViewData

    var body: some View {
        if style.isVisible {
            VStack(spacing: SpacingTokens.small) {
                if !originalText.isEmpty {
                    Text(originalText)
                        .font(.system(size: 18 * style.fontScale, weight: .semibold))
                        .foregroundStyle(ColorTokens.originalSubtitle)
                        .multilineTextAlignment(.center)
                }

                if style.isBilingual && !translatedText.isEmpty {
                    Text(translatedText)
                        .font(.system(size: 16 * style.fontScale, weight: .medium))
                        .foregroundStyle(ColorTokens.translatedSubtitle)
                        .multilineTextAlignment(.center)
                }
            }
            .padding(.horizontal, SpacingTokens.large)
            .padding(.vertical, SpacingTokens.small)
            .background(ColorTokens.subtitleBackground)
            .clipShape(RoundedRectangle(cornerRadius: 16, style: .continuous))
            .padding(.horizontal, SpacingTokens.medium)
            .padding(.bottom, style.verticalOffset)
        }
    }
}
