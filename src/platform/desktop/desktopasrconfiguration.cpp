#include "platform/desktop/desktopasrconfiguration.h"

#include "core/model/modelmanager.h"
#include "core/path/apppaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>

AsrSettings loadDesktopAsrSettings() {
    QSettings settings(QStringLiteral("AIPlayer"), QStringLiteral("Settings"));

    AsrSettings config;
    config.modelIndex = settings.value(QStringLiteral("model_index"), config.modelIndex).toInt();
    config.sourceLanguage = settings.value(QStringLiteral("source_lang"), config.sourceLanguage).toString();
    return config;
}

QString resolveDesktopAsrModelPath(int modelIndex) {
    const ModelManager modelManager;
    const ModelDescriptor descriptor = modelManager.asrModelAt(modelIndex);

    const QStringList candidates = {
        modelManager.filePathFor(descriptor),
        QDir(AppPaths::modelsDirectory()).absoluteFilePath(descriptor.fileName),
        QCoreApplication::applicationDirPath() + QStringLiteral("/") + descriptor.fileName,
        QDir::currentPath() + QStringLiteral("/") + descriptor.fileName,
        QDir::homePath() + QStringLiteral("/") + descriptor.fileName
    };

    for (const QString &path : candidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }

    return QString();
}
