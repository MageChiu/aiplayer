import SwiftUI

struct LibraryScreen: View {
    @ObservedObject var viewModel: LibraryViewModel
    let onSelectItem: (LibraryResourceViewData) -> Void
    let onImportVideo: () -> Void
    let onImportSubtitle: () -> Void

    var body: some View {
        VStack(spacing: SpacingTokens.medium) {
            if let banner = viewModel.statusBannerText {
                StatusBannerView(text: banner, style: viewModel.statusStyle)
                    .padding(.horizontal, SpacingTokens.medium)
                    .padding(.top, SpacingTokens.small)
            }

            Picker("资源分类", selection: $viewModel.selectedSegment) {
                ForEach(LibraryResourceViewData.Kind.allCases, id: \.self) { kind in
                    Text(kind.rawValue).tag(kind)
                }
            }
            .pickerStyle(.segmented)
            .padding(.horizontal, SpacingTokens.medium)

            if viewModel.visibleItems.isEmpty {
                emptyState
            } else {
                List(viewModel.visibleItems) { item in
                    Button {
                        onSelectItem(item)
                    } label: {
                        VStack(alignment: .leading, spacing: 6) {
                            HStack {
                                Text(item.title)
                                    .font(TypographyTokens.cardTitle)
                                Spacer()
                                Text(item.status)
                                    .font(TypographyTokens.caption)
                                    .foregroundStyle(.secondary)
                            }
                            Text(item.detail)
                                .font(TypographyTokens.caption)
                                .foregroundStyle(.secondary)
                        }
                        .padding(.vertical, 4)
                    }
                    .buttonStyle(.plain)
                }
                .listStyle(.automatic)
            }
        }
        .background(ColorTokens.surface)
    }

    private var emptyState: some View {
        VStack(spacing: SpacingTokens.medium) {
            Spacer()
            Image(systemName: "square.stack.3d.up.slash")
                .font(.system(size: 42))
                .foregroundStyle(ColorTokens.accent)
            Text("资源库还是空的")
                .font(TypographyTokens.sectionTitle)
            HStack(spacing: SpacingTokens.medium) {
                Button("导入视频", action: onImportVideo)
                    .buttonStyle(.borderedProminent)
                Button("导入字幕", action: onImportSubtitle)
                    .buttonStyle(.bordered)
            }
            Spacer()
        }
        .frame(maxWidth: .infinity)
    }
}
