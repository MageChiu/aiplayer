import Foundation

struct LibraryResourceViewData: Identifiable, Hashable {
    enum Kind: String, CaseIterable, Hashable {
        case video = "视频"
        case subtitle = "字幕"
        case recent = "最近"
    }

    let id: UUID
    let kind: Kind
    let title: String
    let detail: String
    let status: String
    let resourceID: UUID?
    let localFilePath: String?

    init(
        id: UUID = UUID(),
        kind: Kind,
        title: String,
        detail: String,
        status: String,
        resourceID: UUID? = nil,
        localFilePath: String? = nil
    ) {
        self.id = id
        self.kind = kind
        self.title = title
        self.detail = detail
        self.status = status
        self.resourceID = resourceID
        self.localFilePath = localFilePath
    }
}
