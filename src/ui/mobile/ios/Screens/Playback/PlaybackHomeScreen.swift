import SwiftUI

struct PlaybackHomeScreen: View {
    @EnvironmentObject private var coordinator: AppCoordinator
    @ObservedObject var viewModel: PlaybackViewModel

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: SpacingTokens.large) {
                if let banner = viewModel.statusBannerText {
                    StatusBannerView(text: banner, style: viewModel.statusStyle)
                }

                if let item = viewModel.continueWatchingItem {
                    VStack(alignment: .leading, spacing: SpacingTokens.small) {
                        Text("继续播放")
                            .font(TypographyTokens.sectionTitle)
                        ResourceCardView(item: item) {
                            coordinator.openPlayer(for: item)
                        }
                    }
                }

                quickActions

                if viewModel.recentItems.isEmpty {
                    emptyState
                } else {
                    VStack(alignment: .leading, spacing: SpacingTokens.small) {
                        Text("最近资源")
                            .font(TypographyTokens.sectionTitle)

                        ForEach(viewModel.recentItems) { item in
                            ResourceCardView(item: item) {
                                coordinator.openPlayer(for: item)
                            }
                        }
                    }
                }
            }
            .padding(SpacingTokens.medium)
        }
        .background(ColorTokens.surface)
    }

    private var quickActions: some View {
        VStack(alignment: .leading, spacing: SpacingTokens.small) {
            Text("快速操作")
                .font(TypographyTokens.sectionTitle)

            HStack(spacing: SpacingTokens.medium) {
                quickActionButton(title: "导入视频", systemImage: "square.and.arrow.down") {
                    coordinator.requestImport(.video)
                }

                quickActionButton(title: "加载字幕", systemImage: "captions.bubble") {
                    coordinator.requestImport(.subtitle)
                }

                quickActionButton(title: "开始播放", systemImage: "play.fill") {
                    coordinator.openLatestPlayback()
                }
            }
        }
    }

    private var emptyState: some View {
        VStack(alignment: .center, spacing: SpacingTokens.medium) {
            Image(systemName: "film.stack")
                .font(.system(size: 40))
                .foregroundStyle(ColorTokens.accent)
            Text("导入一个本地视频开始使用")
                .font(TypographyTokens.sectionTitle)
            Text("首版支持本地视频、字幕导入和 AVPlayer 播放链路。")
                .font(TypographyTokens.body)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
            Button("导入视频") {
                coordinator.requestImport(.video)
            }
            .buttonStyle(.borderedProminent)
        }
        .frame(maxWidth: .infinity)
        .padding(SpacingTokens.xLarge)
        .background(ColorTokens.surface)
        .clipShape(RoundedRectangle(cornerRadius: 20, style: .continuous))
    }

    private func quickActionButton(title: String, systemImage: String, action: @escaping () -> Void) -> some View {
        Button {
            action()
        } label: {
            VStack(spacing: SpacingTokens.small) {
                Image(systemName: systemImage)
                    .font(.title2)
                Text(title)
                    .font(TypographyTokens.caption)
            }
            .frame(maxWidth: .infinity)
            .padding(.vertical, SpacingTokens.large)
            .background(ColorTokens.surface)
            .clipShape(RoundedRectangle(cornerRadius: 18, style: .continuous))
        }
        .buttonStyle(.plain)
    }
}
