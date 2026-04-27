#pragma once

#include <QtCore/QList>
#include <QtCore/QString>

enum class ModelKind {
    Asr,
    Translation
};

struct ModelDescriptor {
    QString id;
    QString displayName;
    QString fileName;
    QString downloadUrl;
    ModelKind kind = ModelKind::Asr;
};

class ModelCatalog {
public:
    QList<ModelDescriptor> asrModels() const;
    QList<ModelDescriptor> translationModels() const;

    ModelDescriptor asrModelAt(int index) const;
    ModelDescriptor translationModelAt(int index) const;
    ModelDescriptor asrModelByUrl(const QString &url) const;
    ModelDescriptor translationModelByUrl(const QString &url) const;
};

class IModelPathResolver {
public:
    virtual ~IModelPathResolver() = default;

    virtual QString directoryFor(ModelKind kind) const = 0;
    virtual QString filePathFor(const ModelDescriptor &descriptor) const = 0;
    virtual bool isInstalled(const ModelDescriptor &descriptor) const = 0;
};

class ModelManager {
public:
    explicit ModelManager(const IModelPathResolver *pathResolver = nullptr);

    QList<ModelDescriptor> asrModels() const;
    QList<ModelDescriptor> translationModels() const;

    QString directoryFor(ModelKind kind) const;
    QString filePathFor(const ModelDescriptor &descriptor) const;
    bool isInstalled(const ModelDescriptor &descriptor) const;

    ModelDescriptor asrModelAt(int index) const;
    ModelDescriptor translationModelAt(int index) const;
    ModelDescriptor asrModelByUrl(const QString &url) const;
    ModelDescriptor translationModelByUrl(const QString &url) const;

private:
    ModelCatalog m_catalog;
    const IModelPathResolver *m_pathResolver = nullptr;
};
