import Foundation

@MainActor
final class LibraryViewModel: ObservableObject {
    @Published var selectedSegment: LibraryResourceViewData.Kind = .video
    @Published var items: [LibraryResourceViewData]
    @Published var statusBannerText: String?
    @Published var statusStyle: StatusBannerView.Style

    init(
        items: [LibraryResourceViewData],
        statusBannerText: String? = nil,
        statusStyle: StatusBannerView.Style = .info
    ) {
        self.items = items
        self.statusBannerText = statusBannerText
        self.statusStyle = statusStyle
    }

    var visibleItems: [LibraryResourceViewData] {
        items.filter { item in
            switch selectedSegment {
            case .video:
                return item.kind == .video
            case .subtitle:
                return item.kind == .subtitle
            case .recent:
                return true
            }
        }
    }

    static func makeMock() -> LibraryViewModel {
        LibraryViewModel(
            items: [
                LibraryResourceViewData(kind: .video, title: "Planet Earth Episode 1", detail: "1.4 GB / 最近打开", status: "可播放"),
                LibraryResourceViewData(kind: .video, title: "TED Talk - Learning Faster", detail: "220 MB / 已导入", status: "缺少字幕"),
                LibraryResourceViewData(kind: .subtitle, title: "planet_earth_zh_en.srt", detail: "82 KB / 双语字幕", status: "已关联"),
                LibraryResourceViewData(kind: .recent, title: "Sample Movie Trailer", detail: "昨天 / 最近播放", status: "已完成"),
            ],
            statusBannerText: "资源页用于承载视频、字幕和最近打开项。"
        )
    }
}
