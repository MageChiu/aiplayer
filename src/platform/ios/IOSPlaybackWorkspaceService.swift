import Foundation

final class IOSPlaybackWorkspaceService {
    private let repository: IOSImportedResourceRepository

    private(set) var resources: [ImportedResource] = []

    init(repository: IOSImportedResourceRepository = IOSImportedResourceRepository()) {
        self.repository = repository
    }

    func loadResources() throws {
        resources = try repository.loadResources()
    }

    func importResource(from url: URL, kind: ImportedResourceKind) throws -> ImportedResource {
        let imported = try repository.importResource(from: url, kind: kind)
        resources.insert(imported, at: 0)
        try repository.saveResources(resources)
        return imported
    }

    func resolveURL(for resource: ImportedResource) throws -> URL {
        try repository.resolveURL(for: resource)
    }

    func linkedSubtitle(for resource: ImportedResource) -> ImportedResource? {
        resources.first(where: { $0.id == resource.linkedVideoResourceID })
    }

    func updatePlaybackProgress(for resourceID: UUID, progress: Double, lastOpenedAt: Date) throws {
        guard let index = resources.firstIndex(where: { $0.id == resourceID }) else {
            return
        }
        resources[index].playbackProgress = progress
        resources[index].lastOpenedAt = lastOpenedAt
        try repository.saveResources(resources)
    }
}
