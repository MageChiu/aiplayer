import Foundation

struct PlaybackItemViewData: Identifiable, Hashable {
    let id: UUID
    let resourceID: UUID?
    let title: String
    let detail: String
    let progress: Double
    let subtitleSummary: String
    let translationSummary: String
    let lastOpenedLabel: String
    let thumbnailSystemName: String
    let status: String
    let localFilePath: String?

    init(
        id: UUID = UUID(),
        resourceID: UUID? = nil,
        title: String,
        detail: String,
        progress: Double,
        subtitleSummary: String,
        translationSummary: String,
        lastOpenedLabel: String,
        thumbnailSystemName: String,
        status: String,
        localFilePath: String? = nil
    ) {
        self.id = id
        self.resourceID = resourceID
        self.title = title
        self.detail = detail
        self.progress = progress
        self.subtitleSummary = subtitleSummary
        self.translationSummary = translationSummary
        self.lastOpenedLabel = lastOpenedLabel
        self.thumbnailSystemName = thumbnailSystemName
        self.status = status
        self.localFilePath = localFilePath
    }

    static func sample(
        title: String,
        detail: String,
        progress: Double,
        subtitleSummary: String,
        translationSummary: String,
        status: String
    ) -> PlaybackItemViewData {
        PlaybackItemViewData(
            title: title,
            detail: detail,
            progress: progress,
            subtitleSummary: subtitleSummary,
            translationSummary: translationSummary,
            lastOpenedLabel: "刚刚",
            thumbnailSystemName: "film",
            status: status
        )
    }
}
