import Foundation

protocol IOSFileImportService {
    func beginImport(_ resourceKind: CoreMobileImportResourceKindSnapshot)
}

final class MockIOSFileImportService: IOSFileImportService {
    func beginImport(_ resourceKind: CoreMobileImportResourceKindSnapshot) {}
}
