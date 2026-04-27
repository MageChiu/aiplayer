import Foundation

final class IOSImportedResourceRepository {
    private let fileManager: FileManager
    private let pathService: IOSSandboxPathService
    private let encoder: JSONEncoder
    private let decoder: JSONDecoder

    init(
        fileManager: FileManager = .default,
        pathService: IOSSandboxPathService = IOSSandboxPathService()
    ) {
        self.fileManager = fileManager
        self.pathService = pathService
        self.encoder = JSONEncoder()
        self.decoder = JSONDecoder()
        self.encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        self.encoder.dateEncodingStrategy = .iso8601
        self.decoder.dateDecodingStrategy = .iso8601
    }

    func loadResources() throws -> [ImportedResource] {
        let metadataURL = try pathService.resourceMetadataFileURL()
        guard fileManager.fileExists(atPath: metadataURL.path) else {
            return []
        }
        let data = try Data(contentsOf: metadataURL)
        return try decoder.decode([ImportedResource].self, from: data)
    }

    func saveResources(_ resources: [ImportedResource]) throws {
        let metadataURL = try pathService.resourceMetadataFileURL()
        let data = try encoder.encode(resources)
        try data.write(to: metadataURL, options: .atomic)
    }

    func importResource(from selectedURL: URL, kind: ImportedResourceKind) throws -> ImportedResource {
        let destinationDirectory = try directory(for: kind)
        let extensionSuffix = selectedURL.pathExtension.isEmpty ? "" : ".\(selectedURL.pathExtension)"
        let destinationFileName = UUID().uuidString + extensionSuffix
        let destinationURL = destinationDirectory.appendingPathComponent(destinationFileName)

        let didAccessSecurityScope = selectedURL.startAccessingSecurityScopedResource()
        defer {
            if didAccessSecurityScope {
                selectedURL.stopAccessingSecurityScopedResource()
            }
        }

        if fileManager.fileExists(atPath: destinationURL.path) {
            try fileManager.removeItem(at: destinationURL)
        }

        try fileManager.copyItem(at: selectedURL, to: destinationURL)
        let fileSize = try destinationURL.resourceValues(forKeys: [.fileSizeKey]).fileSize ?? 0

        return ImportedResource(
            kind: kind,
            displayName: selectedURL.deletingPathExtension().lastPathComponent,
            originalFileName: selectedURL.lastPathComponent,
            storedRelativePath: try relativePath(for: destinationURL),
            fileSizeBytes: Int64(fileSize)
        )
    }

    func resolveURL(for resource: ImportedResource) throws -> URL {
        try pathService.applicationSupportDirectory().appendingPathComponent(resource.storedRelativePath, isDirectory: false)
    }

    private func directory(for kind: ImportedResourceKind) throws -> URL {
        switch kind {
        case .video:
            return try pathService.importedVideosDirectory()
        case .subtitle:
            return try pathService.importedSubtitlesDirectory()
        }
    }

    private func relativePath(for url: URL) throws -> String {
        let base = try pathService.applicationSupportDirectory().path + "/"
        let full = url.path
        if full.hasPrefix(base) {
            return String(full.dropFirst(base.count))
        }
        return url.lastPathComponent
    }
}
