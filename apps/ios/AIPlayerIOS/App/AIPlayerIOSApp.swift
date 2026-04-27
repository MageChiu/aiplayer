import SwiftUI

@main
struct AIPlayerIOSApp: App {
    @StateObject private var coordinator = AppCoordinator()

    var body: some Scene {
        WindowGroup {
            RootTabShell()
                .environmentObject(coordinator)
        }
    }
}

private struct RootTabShell: View {
    @EnvironmentObject private var coordinator: AppCoordinator

    private var isFileImporterPresented: Binding<Bool> {
        Binding(
            get: { coordinator.activeImportRequest != nil },
            set: { isPresented in
                if !isPresented {
                    coordinator.cancelPendingImport()
                }
            }
        )
    }

    var body: some View {
        TabView(selection: $coordinator.selectedTab) {
            NavigationStack {
                PlaybackHomeScreen(viewModel: coordinator.homePlaybackViewModel)
                    .navigationTitle("播放")
                    .toolbar {
                        ToolbarItem(placement: .primaryAction) {
                            Button("导入") {
                                coordinator.startImportFlow()
                            }
                        }
                    }
            }
            .tabItem {
                Label("播放", systemImage: "play.square.stack")
            }
            .tag(AppTab.playback)

            NavigationStack {
                LibraryScreen(
                    viewModel: coordinator.libraryViewModel,
                    onSelectItem: coordinator.handleLibrarySelection,
                    onImportVideo: { coordinator.requestImport(.video) },
                    onImportSubtitle: { coordinator.requestImport(.subtitle) }
                )
                .navigationTitle("资源")
                .toolbar {
                    ToolbarItem(placement: .primaryAction) {
                        Button("导入") {
                            coordinator.startImportFlow()
                        }
                    }
                }
            }
            .tabItem {
                Label("资源", systemImage: "square.stack.3d.up")
            }
            .tag(AppTab.library)

            NavigationStack {
                SettingsScreen(viewModel: coordinator.settingsViewModel)
                    .navigationTitle("设置")
            }
            .tabItem {
                Label("设置", systemImage: "gearshape")
            }
            .tag(AppTab.settings)
        }
        .tint(ColorTokens.accent)
        .sheet(isPresented: $coordinator.isImportSheetPresented) {
            ImportEntrySheet(
                onImportVideo: { coordinator.requestImport(.video) },
                onImportSubtitle: { coordinator.requestImport(.subtitle) },
                onDismiss: coordinator.dismissImportFlow
            )
            .presentationDetents([.medium])
        }
        .fileImporter(
            isPresented: isFileImporterPresented,
            allowedContentTypes: coordinator.allowedImportTypes,
            allowsMultipleSelection: false,
            onCompletion: coordinator.handleImportSelection
        )
#if os(iOS)
        .fullScreenCover(item: $coordinator.activePlayerViewModel) { viewModel in
            PlayerScreen(
                viewModel: viewModel,
                onDismiss: coordinator.dismissPlayer
            )
        }
#else
        .sheet(item: $coordinator.activePlayerViewModel) { viewModel in
            PlayerScreen(
                viewModel: viewModel,
                onDismiss: coordinator.dismissPlayer
            )
        }
#endif
    }
}
