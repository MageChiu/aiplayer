import Foundation
import UniformTypeIdentifiers

@MainActor
final class AppCoordinator: ObservableObject {
    @Published var selectedTab: AppTab = .playback {
        didSet { syncNavigationState() }
    }
    @Published var isImportSheetPresented = false
    @Published var activeImportRequest: FileImportRequest?
    @Published var activePlayerViewModel: PlayerViewModel? {
        didSet { syncNavigationState() }
    }
    @Published var homePlaybackViewModel = PlaybackViewModel.makeHomeMock()
    @Published var libraryViewModel = LibraryViewModel.makeMock()
    @Published var settingsViewModel = SettingsViewModel.makeMock()
    @Published var mobileShellState = CoreMobileShellStateSnapshot(
        launchStage: .initializing,
        lifecycle: .foreground,
        navigation: CoreNavigationStateSnapshot(currentRoute: .player, canGoBack: false),
        importState: .idle
    )

    private let workspaceService = IOSPlaybackWorkspaceService()

    init() {
        loadPersistedResources()
    }

    var allowedImportTypes: [UTType] {
        activeImportRequest?.allowedContentTypes ?? [.data]
    }

    func startImportFlow() {
        isImportSheetPresented = true
        mobileShellState.importState = .loading
    }

    func dismissImportFlow() {
        isImportSheetPresented = false
        mobileShellState.importState = .idle
    }

    func requestImport(_ request: FileImportRequest) {
        activeImportRequest = request
        isImportSheetPresented = false
        mobileShellState.importState = .loading
    }

    func cancelPendingImport() {
        activeImportRequest = nil
        mobileShellState.importState = .idle
    }

    func handleImportSelection(_ result: Result<[URL], Error>) {
        guard let request = activeImportRequest else {
            return
        }

        activeImportRequest = nil

        switch result {
        case .success(let urls):
            guard let selectedURL = urls.first else {
                broadcastStatus("未选择任何文件。", style: .warning)
                return
            }
            importResource(from: selectedURL, request: request)
        case .failure(let error):
            broadcastStatus("文件导入失败：\(error.localizedDescription)", style: .error)
        }
    }

    func openLatestPlayback() {
        if let item = homePlaybackViewModel.continueWatchingItem ?? homePlaybackViewModel.recentItems.first {
            openPlayer(for: item)
        }
    }

    func openPlayer(for item: PlaybackItemViewData) {
        guard let resourceID = item.resourceID,
              let resource = workspaceService.resources.first(where: { $0.id == resourceID }) else {
            broadcastStatus("当前资源仅为示例数据，尚未绑定真实文件。", style: .warning)
            return
        }
        openPlayer(for: resource)
    }

    func handleLibrarySelection(_ item: LibraryResourceViewData) {
        guard let resourceID = item.resourceID,
              let resource = workspaceService.resources.first(where: { $0.id == resourceID }) else {
            broadcastStatus("当前条目尚未绑定真实资源。", style: .warning)
            return
        }

        switch resource.kind {
        case .video:
            openPlayer(for: resource)
        case .subtitle:
            broadcastStatus("字幕已导入，可在后续版本中绑定到指定视频。", style: .info)
        }
    }

    func dismissPlayer() {
        guard let activePlayerViewModel else {
            return
        }

        activePlayerViewModel.prepareForDismissal()
        updatePlaybackProgress(
            for: activePlayerViewModel.resource.id,
            progress: activePlayerViewModel.captureProgress(),
            lastOpenedAt: Date()
        )
        self.activePlayerViewModel = nil
    }

    private func importResource(from url: URL, request: FileImportRequest) {
        do {
            let imported = try workspaceService.importResource(from: url, kind: request.resourceKind)
            rebuildViewModels(statusText: request.successMessage(for: imported.displayName), style: .success)
            mobileShellState.importState = .ready

            if imported.kind == .video {
                openPlayer(for: imported)
            }
        } catch {
            mobileShellState.importState = .failed
            broadcastStatus("导入失败：\(error.localizedDescription)", style: .error)
        }
    }

    private func openPlayer(for resource: ImportedResource) {
        do {
            let sourceURL = try workspaceService.resolveURL(for: resource)
            let linkedSubtitle = workspaceService.linkedSubtitle(for: resource)
            let viewModel = PlayerViewModel(
                resource: resource,
                sourceURL: sourceURL,
                linkedSubtitle: linkedSubtitle
            )
            activePlayerViewModel = viewModel
            updatePlaybackProgress(for: resource.id, progress: resource.playbackProgress, lastOpenedAt: Date())
        } catch {
            broadcastStatus("无法打开资源：\(error.localizedDescription)", style: .error)
        }
    }

    private func loadPersistedResources() {
        do {
            try workspaceService.loadResources()
            mobileShellState.launchStage = .ready
            mobileShellState.importState = workspaceService.resources.isEmpty ? .empty : .ready
            rebuildViewModels(statusText: workspaceService.resources.isEmpty ? "iOS 应用骨架已就绪，请先导入本地视频。" : "已恢复本地资源与最近播放记录。", style: .info)
        } catch {
            mobileShellState.launchStage = .error
            mobileShellState.importState = .failed
            rebuildViewModels(statusText: "读取本地资源失败：\(error.localizedDescription)", style: .warning)
        }
    }

    private func updatePlaybackProgress(for resourceID: UUID, progress: Double, lastOpenedAt: Date) {
        do {
            try workspaceService.updatePlaybackProgress(for: resourceID, progress: progress, lastOpenedAt: lastOpenedAt)
            rebuildViewModels(statusText: "已保存播放进度。", style: .info)
        } catch {
            broadcastStatus("保存播放进度失败：\(error.localizedDescription)", style: .warning)
        }
    }

    private func rebuildViewModels(statusText: String?, style: StatusBannerView.Style) {
        let videoResources = workspaceService.resources
            .filter { $0.kind == .video }
            .sorted { lhs, rhs in
                let leftDate = lhs.lastOpenedAt ?? lhs.importedAt
                let rightDate = rhs.lastOpenedAt ?? rhs.importedAt
                return leftDate > rightDate
            }

        let playbackItems = videoResources.map(makePlaybackItem)
        homePlaybackViewModel = PlaybackViewModel(
            recentItems: playbackItems,
            statusBannerText: statusText,
            statusStyle: style
        )

        let libraryItems = workspaceService.resources
            .sorted { ($0.lastOpenedAt ?? $0.importedAt) > ($1.lastOpenedAt ?? $1.importedAt) }
            .map(makeLibraryItem)
        libraryViewModel = LibraryViewModel(
            items: libraryItems,
            statusBannerText: statusText,
            statusStyle: style
        )
        syncNavigationState()
    }

    private func broadcastStatus(_ text: String, style: StatusBannerView.Style) {
        rebuildViewModels(statusText: text, style: style)
    }

    private func makePlaybackItem(from resource: ImportedResource) -> PlaybackItemViewData {
        PlaybackItemViewData(
            resourceID: resource.id,
            title: resource.displayName,
            detail: resource.detailLine,
            progress: resource.playbackProgress,
            subtitleSummary: resource.linkedVideoResourceID == nil ? "字幕待关联" : "字幕已关联",
            translationSummary: "在线翻译待接入",
            lastOpenedLabel: resource.lastOpenedLabel,
            thumbnailSystemName: "film",
            status: resource.playbackProgress > 0 && resource.playbackProgress < 0.98 ? "继续播放" : "可播放",
            localFilePath: resource.storedRelativePath
        )
    }

    private func makeLibraryItem(from resource: ImportedResource) -> LibraryResourceViewData {
        LibraryResourceViewData(
            kind: resource.kind == .video ? .video : .subtitle,
            title: resource.displayName,
            detail: resource.detailLine,
            status: resource.kind == .video ? "可打开" : "已导入",
            resourceID: resource.id,
            localFilePath: resource.storedRelativePath
        )
    }

    private func syncNavigationState() {
        let route: CoreAppRouteSnapshot
        if activePlayerViewModel != nil {
            route = .player
        } else {
            route = selectedTab == .settings ? .settings : (selectedTab == .library ? .browser : .player)
        }
        mobileShellState.navigation = CoreNavigationStateSnapshot(
            currentRoute: route,
            canGoBack: activePlayerViewModel != nil || selectedTab != .playback
        )
    }
}

enum AppTab: Hashable {
    case playback
    case library
    case settings
}
