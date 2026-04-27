import Foundation
import UniformTypeIdentifiers

enum FileImportRequest: String, Identifiable {
    case video
    case subtitle

    var id: String { rawValue }

    var coreResourceKind: CoreMobileImportResourceKindSnapshot {
        switch self {
        case .video:
            return .video
        case .subtitle:
            return .subtitle
        }
    }

    var resourceKind: ImportedResourceKind {
        switch self {
        case .video:
            return .video
        case .subtitle:
            return .subtitle
        }
    }

    var allowedContentTypes: [UTType] {
        switch self {
        case .video:
            return [.movie, .video, .mpeg4Movie, .quickTimeMovie]
        case .subtitle:
            let extensions = ["srt", "ass", "ssa", "vtt", "sub"]
            let customTypes = extensions.compactMap { extensionName in
                UTType(filenameExtension: extensionName)
            }
            return customTypes + [.plainText]
        }
    }

    func successMessage(for fileName: String) -> String {
        switch self {
        case .video:
            return "已导入视频：\(fileName)"
        case .subtitle:
            return "已导入字幕：\(fileName)"
        }
    }
}
