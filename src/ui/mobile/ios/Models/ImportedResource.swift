import Foundation

enum ImportedResourceKind: String, Codable, Hashable {
    case video
    case subtitle
}

struct ImportedResource: Identifiable, Codable, Hashable {
    let id: UUID
    let kind: ImportedResourceKind
    let displayName: String
    let originalFileName: String
    let storedRelativePath: String
    let fileSizeBytes: Int64
    let importedAt: Date
    var lastOpenedAt: Date?
    var playbackProgress: Double
    var linkedVideoResourceID: UUID?

    init(
        id: UUID = UUID(),
        kind: ImportedResourceKind,
        displayName: String,
        originalFileName: String,
        storedRelativePath: String,
        fileSizeBytes: Int64,
        importedAt: Date = Date(),
        lastOpenedAt: Date? = nil,
        playbackProgress: Double = 0.0,
        linkedVideoResourceID: UUID? = nil
    ) {
        self.id = id
        self.kind = kind
        self.displayName = displayName
        self.originalFileName = originalFileName
        self.storedRelativePath = storedRelativePath
        self.fileSizeBytes = fileSizeBytes
        self.importedAt = importedAt
        self.lastOpenedAt = lastOpenedAt
        self.playbackProgress = playbackProgress
        self.linkedVideoResourceID = linkedVideoResourceID
    }

    var detailLine: String {
        let sizeText = ByteCountFormatter.string(fromByteCount: fileSizeBytes, countStyle: .file)
        return "\(sizeText) / \(lastOpenedLabel)"
    }

    var lastOpenedLabel: String {
        let formatter = RelativeDateTimeFormatter()
        formatter.locale = Locale(identifier: "zh_CN")
        return formatter.localizedString(for: lastOpenedAt ?? importedAt, relativeTo: Date())
    }
}
