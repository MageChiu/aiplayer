import SwiftUI

struct SettingsScreen: View {
    @ObservedObject var viewModel: SettingsViewModel

    private let supportedLanguages = ["简体中文", "English", "日本語"]

    var body: some View {
        Form {
            Section("播放") {
                Picker("默认倍速", selection: $viewModel.defaultPlaybackSpeed) {
                    Text("1.0x").tag(1.0)
                    Text("1.25x").tag(1.25)
                    Text("1.5x").tag(1.5)
                }
                Toggle("自动续播", isOn: $viewModel.autoResumeEnabled)
                Toggle("旋转后自动全屏", isOn: $viewModel.autoFullscreenOnRotate)
            }

            Section("字幕") {
                Toggle("默认双语显示", isOn: $viewModel.defaultBilingualSubtitle)
                NavigationLink("默认样式") {
                    List {
                        Text("后续接入字幕样式编辑器")
                    }
                    .navigationTitle("字幕样式")
                }
            }

            Section("翻译") {
                Picker("默认目标语言", selection: $viewModel.defaultTargetLanguage) {
                    ForEach(supportedLanguages, id: \.self) { language in
                        Text(language).tag(language)
                    }
                }
                Toggle("仅 Wi-Fi 下启用在线翻译", isOn: $viewModel.wifiOnlyTranslation)
            }

            Section("诊断") {
                Toggle("启用基础诊断信息", isOn: $viewModel.diagnosticsEnabled)
                LabeledContent("版本", value: "v-i-1.0.0")
            }
        }
    }
}
