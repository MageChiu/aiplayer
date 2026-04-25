#pragma once

#include "core/path/ipathprovider.h"
#include "platform/mobile/mobilecontracts.h"

#include <QDir>

#include <utility>

class MobilePathProvider : public IPathProvider {
public:
    explicit MobilePathProvider(QString rootDirectory = QDir::homePath() + QStringLiteral("/.aiplayer-mobile"))
        : m_rootDirectory(std::move(rootDirectory)) {}

    QString appDataDirectory() const override { return m_rootDirectory; }
    QString modelsDirectory() const override { return QDir(appDataDirectory()).absoluteFilePath(QStringLiteral("models")); }
    QString asrModelsDirectory() const override { return QDir(modelsDirectory()).absoluteFilePath(QStringLiteral("asr")); }
    QString translationModelsDirectory() const override { return QDir(modelsDirectory()).absoluteFilePath(QStringLiteral("translation")); }
    QString torrentsDirectory() const override { return QDir(appDataDirectory()).absoluteFilePath(QStringLiteral("torrents")); }
    QString importedResourcesDirectory() const { return QDir(appDataDirectory()).absoluteFilePath(QStringLiteral("imports")); }
    QString importedResourceDirectory(MobileImportResourceKind resourceKind) const {
        const auto directoryName = resourceKind == MobileImportResourceKind::Subtitle
            ? QStringLiteral("subtitles")
            : QStringLiteral("videos");
        return QDir(importedResourcesDirectory()).absoluteFilePath(directoryName);
    }

private:
    QString m_rootDirectory;
};
