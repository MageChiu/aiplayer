#pragma once

#include "core/model/modelmanager.h"

#include <QObject>

class ModelDownloadService;

class DesktopModelCoordinator : public QObject {
    Q_OBJECT

public:
    explicit DesktopModelCoordinator(QObject *parent = nullptr);

    const ModelManager &modelManager() const;
    ModelDownloadService *downloadService() const;

    QString directoryFor(ModelKind kind) const;
    QString filePathFor(const ModelDescriptor &descriptor) const;
    bool isInstalled(const ModelDescriptor &descriptor) const;
    bool openDirectory(ModelKind kind, QString *errorMessage = nullptr) const;

private:
    ModelManager m_modelManager;
    ModelDownloadService *m_downloadService = nullptr;
};
