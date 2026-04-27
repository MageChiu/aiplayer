import SwiftUI

struct ResourceCardView: View {
    let item: PlaybackItemViewData
    let action: () -> Void

    var body: some View {
        Button {
            action()
        } label: {
            HStack(spacing: SpacingTokens.medium) {
                Image(systemName: item.thumbnailSystemName)
                    .font(.system(size: 30))
                    .foregroundStyle(ColorTokens.accent)
                    .frame(width: 54, height: 54)
                    .background(ColorTokens.elevatedSurface)
                    .clipShape(RoundedRectangle(cornerRadius: 14, style: .continuous))

                VStack(alignment: .leading, spacing: SpacingTokens.xSmall) {
                    HStack {
                        Text(item.title)
                            .font(TypographyTokens.cardTitle)
                            .foregroundStyle(.primary)
                            .lineLimit(1)
                        Spacer(minLength: 0)
                        Text(item.status)
                            .font(TypographyTokens.caption)
                            .padding(.horizontal, 8)
                            .padding(.vertical, 4)
                            .background(ColorTokens.accent.opacity(0.12))
                            .clipShape(Capsule())
                    }

                    Text(item.detail)
                        .font(TypographyTokens.caption)
                        .foregroundStyle(.secondary)

                    ProgressView(value: item.progress)
                        .tint(ColorTokens.accent)

                    Text(item.subtitleSummary + " · " + item.translationSummary)
                        .font(TypographyTokens.caption)
                        .foregroundStyle(.secondary)
                        .multilineTextAlignment(.leading)
                }
            }
            .padding(SpacingTokens.medium)
            .background(ColorTokens.surface)
            .clipShape(RoundedRectangle(cornerRadius: 20, style: .continuous))
        }
        .buttonStyle(.plain)
    }
}
