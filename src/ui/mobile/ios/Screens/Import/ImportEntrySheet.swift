import SwiftUI

struct ImportEntrySheet: View {
    let onImportVideo: () -> Void
    let onImportSubtitle: () -> Void
    let onDismiss: () -> Void

    var body: some View {
        NavigationStack {
            VStack(alignment: .leading, spacing: SpacingTokens.large) {
                Text("选择导入类型")
                    .font(TypographyTokens.heroTitle)

                Text("当前版本会打开系统文件选择器，并将选中的视频或字幕复制到应用沙盒目录。")
                    .font(TypographyTokens.body)
                    .foregroundStyle(.secondary)

                Button(action: onImportVideo) {
                    Label("导入视频", systemImage: "film.stack")
                        .frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)

                Button(action: onImportSubtitle) {
                    Label("导入字幕", systemImage: "captions.bubble")
                        .frame(maxWidth: .infinity)
                }
                .buttonStyle(.bordered)

                Spacer()
            }
            .padding(SpacingTokens.large)
            .navigationTitle("导入")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("关闭", action: onDismiss)
                }
            }
        }
    }
}
