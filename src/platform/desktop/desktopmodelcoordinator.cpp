#include "platform/desktop/desktopmodelcoordinator.h"

#include "libs/model/include/modeldownloadservice.h"

#include <QDesktopServices>
#include <QDir>
#include <QUrl>

DesktopModelCoordinator::DesktopModelCoordinator(QObject *parent)
    : QObject(parent), m_downloadService(new ModelDownloadService(this)) {
}

const ModelManager &DesktopModelCoordinator::modelManager() const {
    return m_modelManager;
}

ModelDownloadService *DesktopModelCoordinator::downloadService() const {
    return m_downloadService;
}

QString DesktopModelCoordinator::directoryFor(ModelKind kind) const {
    return m_modelManager.directoryFor(kind);
}

QString DesktopModelCoordinator::filePathFor(const ModelDescriptor &descriptor) const {
    return m_modelManager.filePathFor(descriptor);
}

bool DesktopModelCoordinator::isInstalled(const ModelDescriptor &descriptor) const {
    return m_modelManager.isInstalled(descriptor);
}

bool DesktopModelCoordinator::openDirectory(ModelKind kind, QString *errorMessage) const {
    const QString dirPath = directoryFor(kind);
    QDir().mkpath(dirPath);
    const bool opened = QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
    if (!opened && errorMessage) {
        *errorMessage = QStringLiteral("无法打开模型目录：%1").arg(dirPath);
    }
    return opened;
}
