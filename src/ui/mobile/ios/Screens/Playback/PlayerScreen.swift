import SwiftUI

struct PlayerScreen: View {
    @ObservedObject var viewModel: PlayerViewModel
    let onDismiss: () -> Void

    @State private var isSubtitleSheetPresented = false
    @State private var isTranslationSheetPresented = false

    var body: some View {
        ZStack {
            ColorTokens.playerBackground
                .ignoresSafeArea()

            VStack(spacing: 0) {
                header

                ZStack(alignment: .bottom) {
                    AVPlayerSurfaceView(player: viewModel.player)
                        .overlay(alignment: .topLeading) {
                            VStack(alignment: .leading, spacing: SpacingTokens.xSmall) {
                                Text(viewModel.resource.displayName)
                                    .font(TypographyTokens.cardTitle)
                                Text(viewModel.coreStateSnapshot.subtitles.statusMessage)
                                    .font(TypographyTokens.caption)
                            }
                            .foregroundStyle(.white)
                            .padding(SpacingTokens.medium)
                        }
                        .padding(.horizontal, SpacingTokens.medium)
                        .padding(.top, SpacingTokens.small)

                    SubtitleOverlayView(
                        originalText: viewModel.subtitleOriginalText,
                        translatedText: viewModel.subtitleTranslatedText,
                        style: viewModel.subtitleStyle
                    )
                }

                if viewModel.isBuffering {
                    ProgressView("正在缓冲")
                        .tint(.white)
                        .padding(.top, SpacingTokens.small)
                }

                VStack(spacing: SpacingTokens.medium) {
                    PlayerControlsView(
                        progress: $viewModel.progress,
                        isPlaying: viewModel.isPlaying,
                        elapsedText: viewModel.elapsedText,
                        durationText: viewModel.durationText,
                        playbackSpeed: viewModel.playbackSpeed,
                        isMuted: viewModel.isMuted,
                        onTogglePlayPause: viewModel.togglePlayPause,
                        onSeekBackward: viewModel.seekBackward,
                        onSeekForward: viewModel.seekForward,
                        onToggleMute: viewModel.toggleMute,
                        onSpeedTap: viewModel.cycleSpeed
                    )

                    HStack(spacing: SpacingTokens.medium) {
                        actionChip(title: viewModel.subtitleStyle.isVisible ? "字幕开" : "字幕关", systemImage: "captions.bubble") {
                            isSubtitleSheetPresented = true
                        }

                        actionChip(title: viewModel.translationOption.isEnabled ? "翻译开" : "翻译关", systemImage: "globe") {
                            isTranslationSheetPresented = true
                        }

                        actionChip(title: viewModel.translationOption.targetLanguage, systemImage: "text.bubble") {
                            isTranslationSheetPresented = true
                        }
                    }
                }
                .padding(SpacingTokens.medium)
            }
        }
        .sheet(isPresented: $isSubtitleSheetPresented) {
            subtitleSheet
                .presentationDetents([.medium])
        }
        .sheet(isPresented: $isTranslationSheetPresented) {
            translationSheet
                .presentationDetents([.medium])
        }
    }

    private var header: some View {
        HStack {
            Button {
                onDismiss()
            } label: {
                Image(systemName: "chevron.backward")
                    .font(.title3.weight(.semibold))
            }

            Spacer()

            VStack(spacing: 2) {
                Text(viewModel.resource.displayName)
                    .font(TypographyTokens.cardTitle)
                    .foregroundStyle(.white)
                Text(viewModel.translationOption.statusText)
                    .font(TypographyTokens.caption)
                    .foregroundStyle(.white.opacity(0.72))
            }

            Spacer()

            Menu {
                Button("重新载入资源") {}
                Button("查看状态快照") {}
            } label: {
                Image(systemName: "ellipsis.circle")
                    .font(.title2)
            }
        }
        .foregroundStyle(.white)
        .padding(.horizontal, SpacingTokens.medium)
        .padding(.top, SpacingTokens.small)
    }

    private func actionChip(title: String, systemImage: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            Label(title, systemImage: systemImage)
                .font(TypographyTokens.caption)
                .frame(maxWidth: .infinity)
        }
        .buttonStyle(.bordered)
        .tint(.white)
    }

    private var subtitleSheet: some View {
        NavigationStack {
            Form {
                Toggle("显示字幕", isOn: $viewModel.subtitleStyle.isVisible)
                Toggle("双语显示", isOn: $viewModel.subtitleStyle.isBilingual)
                VStack(alignment: .leading) {
                    Text("字号")
                    Slider(value: $viewModel.subtitleStyle.fontScale, in: 0.8...1.4)
                }
            }
            .navigationTitle("字幕设置")
        }
    }

    private var translationSheet: some View {
        NavigationStack {
            Form {
                Toggle("启用在线翻译", isOn: Binding(
                    get: { viewModel.translationOption.isEnabled },
                    set: { isEnabled in
                        viewModel.translationOption.isEnabled = isEnabled
                    }
                ))
                LabeledContent("目标语言", value: viewModel.translationOption.targetLanguage)
                LabeledContent("显示模式", value: viewModel.translationOption.displayMode)
                LabeledContent("翻译状态", value: viewModel.translationOption.statusText)
            }
            .navigationTitle("翻译设置")
        }
    }
}
