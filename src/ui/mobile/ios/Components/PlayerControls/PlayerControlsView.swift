import SwiftUI

struct PlayerControlsView: View {
    @Binding var progress: Double

    let isPlaying: Bool
    let elapsedText: String
    let durationText: String
    let playbackSpeed: Double
    let isMuted: Bool
    let onTogglePlayPause: () -> Void
    let onSeekBackward: () -> Void
    let onSeekForward: () -> Void
    let onToggleMute: () -> Void
    let onSpeedTap: () -> Void

    var body: some View {
        VStack(spacing: SpacingTokens.medium) {
            VStack(spacing: SpacingTokens.xSmall) {
                Slider(value: $progress, in: 0...1)
                    .tint(ColorTokens.accent)

                HStack {
                    Text(elapsedText)
                    Spacer()
                    Text(durationText)
                }
                .font(TypographyTokens.monoCaption)
                .foregroundStyle(.secondary)
            }

            HStack(spacing: SpacingTokens.large) {
                Button(action: onSeekBackward) {
                    Image(systemName: "gobackward.10")
                }

                Button(action: onTogglePlayPause) {
                    Image(systemName: isPlaying ? "pause.circle.fill" : "play.circle.fill")
                        .font(.system(size: 42))
                }

                Button(action: onSeekForward) {
                    Image(systemName: "goforward.10")
                }

                Button(action: onSpeedTap) {
                    Text(String(format: "%.2gx", playbackSpeed))
                        .font(TypographyTokens.cardTitle)
                        .frame(minWidth: 52)
                }

                Button(action: onToggleMute) {
                    Image(systemName: isMuted ? "speaker.slash.fill" : "speaker.wave.2.fill")
                }
            }
            .font(.title3)
            .buttonStyle(.bordered)
        }
        .padding(SpacingTokens.medium)
        .background(.ultraThinMaterial)
        .clipShape(RoundedRectangle(cornerRadius: 20, style: .continuous))
    }
}
