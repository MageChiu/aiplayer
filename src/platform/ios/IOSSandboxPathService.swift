import Foundation

final class IOSSandboxPathService {
    private let fileManager: FileManager

    init(fileManager: FileManager = .default) {
        self.fileManager = fileManager
    }

    func applicationSupportDirectory() throws -> URL {
        guard let baseURL = fileManager.urls(for: .applicationSupportDirectory, in: .userDomainMask).first else {
            throw CocoaError(.fileNoSuchFile)
        }
        let directory = baseURL.appendingPathComponent("AIPlayerIOS", isDirectory: true)
        try ensureDirectoryExists(at: directory)
        return directory
    }

    func importedVideosDirectory() throws -> URL {
        let directory = try applicationSupportDirectory().appendingPathComponent("ImportedVideos", isDirectory: true)
        try ensureDirectoryExists(at: directory)
        return directory
    }

    func importedSubtitlesDirectory() throws -> URL {
        let directory = try applicationSupportDirectory().appendingPathComponent("ImportedSubtitles", isDirectory: true)
        try ensureDirectoryExists(at: directory)
        return directory
    }

    func resourceMetadataFileURL() throws -> URL {
        try applicationSupportDirectory().appendingPathComponent("resources.json", isDirectory: false)
    }

    private func ensureDirectoryExists(at url: URL) throws {
        try fileManager.createDirectory(at: url, withIntermediateDirectories: true)
    }
}
