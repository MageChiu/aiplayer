#include "platform/desktop/desktoppathprovider.h"

#include <QDir>
#include <QStandardPaths>

QString DesktopPathProvider::appDataDirectory() const {
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.isEmpty()) {
        return QDir::homePath() + QStringLiteral("/.aiplayer");
    }
    return baseDir;
}

QString DesktopPathProvider::modelsDirectory() const {
    return QDir(appDataDirectory()).absoluteFilePath(QStringLiteral("models"));
}

QString DesktopPathProvider::asrModelsDirectory() const {
    return QDir(modelsDirectory()).absoluteFilePath(QStringLiteral("asr"));
}

QString DesktopPathProvider::translationModelsDirectory() const {
    return QDir(modelsDirectory()).absoluteFilePath(QStringLiteral("translation"));
}

QString DesktopPathProvider::torrentsDirectory() const {
    return QDir(appDataDirectory()).absoluteFilePath(QStringLiteral("torrents"));
}
