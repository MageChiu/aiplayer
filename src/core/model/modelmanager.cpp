#include "core/model/modelmanager.h"

#include "core/path/apppaths.h"

#include <QFileInfo>
#include <QUrl>

namespace {
class DefaultModelPathResolver : public IModelPathResolver {
public:
    QString directoryFor(ModelKind kind) const override {
        return kind == ModelKind::Asr ? AppPaths::asrModelsDirectory() : AppPaths::translationModelsDirectory();
    }

    QString filePathFor(const ModelDescriptor &descriptor) const override {
        return AppPaths::modelFilePathForUrl(directoryFor(descriptor.kind), descriptor.downloadUrl);
    }

    bool isInstalled(const ModelDescriptor &descriptor) const override {
        return QFileInfo::exists(filePathFor(descriptor));
    }
};

QList<ModelDescriptor> buildAsrModels() {
    return {
        {QStringLiteral("whisper-tiny"), QStringLiteral("Tiny (ggml-tiny.bin)"), QStringLiteral("ggml-tiny.bin"), QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.bin"), ModelKind::Asr},
        {QStringLiteral("whisper-base"), QStringLiteral("Base (ggml-base.bin)"), QStringLiteral("ggml-base.bin"), QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin"), ModelKind::Asr},
        {QStringLiteral("whisper-small"), QStringLiteral("Small (ggml-small.bin)"), QStringLiteral("ggml-small.bin"), QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin"), ModelKind::Asr},
        {QStringLiteral("whisper-medium"), QStringLiteral("Medium (ggml-medium.bin)"), QStringLiteral("ggml-medium.bin"), QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-medium.bin"), ModelKind::Asr},
        {QStringLiteral("whisper-large-v3"), QStringLiteral("Large V3 (ggml-large-v3.bin)"), QStringLiteral("ggml-large-v3.bin"), QStringLiteral("https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-large-v3.bin"), ModelKind::Asr},
    };
}

QList<ModelDescriptor> buildTranslationModels() {
    return {
        {QStringLiteral("qwen25-15b-q4km"), QStringLiteral("Qwen2.5 1.5B Instruct Q4_K_M"), QStringLiteral("Qwen2.5-1.5B-Instruct-Q4_K_M.gguf"), QStringLiteral("https://huggingface.co/bartowski/Qwen2.5-1.5B-Instruct-GGUF/resolve/main/Qwen2.5-1.5B-Instruct-Q4_K_M.gguf"), ModelKind::Translation},
        {QStringLiteral("qwen25-3b-q4km"), QStringLiteral("Qwen2.5 3B Instruct Q4_K_M"), QStringLiteral("Qwen2.5-3B-Instruct-Q4_K_M.gguf"), QStringLiteral("https://huggingface.co/bartowski/Qwen2.5-3B-Instruct-GGUF/resolve/main/Qwen2.5-3B-Instruct-Q4_K_M.gguf"), ModelKind::Translation},
        {QStringLiteral("llama32-3b-q4km"), QStringLiteral("Llama 3.2 3B Instruct Q4_K_M"), QStringLiteral("Llama-3.2-3B-Instruct-Q4_K_M.gguf"), QStringLiteral("https://huggingface.co/bartowski/Llama-3.2-3B-Instruct-GGUF/resolve/main/Llama-3.2-3B-Instruct-Q4_K_M.gguf"), ModelKind::Translation},
    };
}

ModelDescriptor modelAtOrFallback(const QList<ModelDescriptor> &models, int index) {
    if (index >= 0 && index < models.size()) {
        return models.at(index);
    }
    return models.isEmpty() ? ModelDescriptor{} : models.first();
}

ModelDescriptor modelByUrlOrFallback(const QList<ModelDescriptor> &models, const QString &url) {
    for (const ModelDescriptor &descriptor : models) {
        if (descriptor.downloadUrl == url) {
            return descriptor;
        }
    }

    if (url.isEmpty()) {
        return models.isEmpty() ? ModelDescriptor{} : models.first();
    }

    ModelDescriptor descriptor;
    descriptor.kind = models.isEmpty() ? ModelKind::Asr : models.first().kind;
    descriptor.downloadUrl = url;
    descriptor.fileName = QUrl(url).fileName();
    descriptor.displayName = descriptor.fileName;
    descriptor.id = descriptor.fileName;
    return descriptor;
}

const IModelPathResolver &fallbackPathResolver() {
    static const DefaultModelPathResolver resolver;
    return resolver;
}
}

ModelManager::ModelManager(const IModelPathResolver *pathResolver)
    : m_pathResolver(pathResolver ? pathResolver : &fallbackPathResolver()) {
}

QList<ModelDescriptor> ModelCatalog::asrModels() const {
    return buildAsrModels();
}

QList<ModelDescriptor> ModelCatalog::translationModels() const {
    return buildTranslationModels();
}

ModelDescriptor ModelCatalog::asrModelAt(int index) const {
    return modelAtOrFallback(asrModels(), index);
}

ModelDescriptor ModelCatalog::translationModelAt(int index) const {
    return modelAtOrFallback(translationModels(), index);
}

ModelDescriptor ModelCatalog::asrModelByUrl(const QString &url) const {
    return modelByUrlOrFallback(asrModels(), url);
}

ModelDescriptor ModelCatalog::translationModelByUrl(const QString &url) const {
    return modelByUrlOrFallback(translationModels(), url);
}

QList<ModelDescriptor> ModelManager::asrModels() const {
    return m_catalog.asrModels();
}

QList<ModelDescriptor> ModelManager::translationModels() const {
    return m_catalog.translationModels();
}

QString ModelManager::directoryFor(ModelKind kind) const {
    return m_pathResolver->directoryFor(kind);
}

QString ModelManager::filePathFor(const ModelDescriptor &descriptor) const {
    return m_pathResolver->filePathFor(descriptor);
}

bool ModelManager::isInstalled(const ModelDescriptor &descriptor) const {
    return m_pathResolver->isInstalled(descriptor);
}

ModelDescriptor ModelManager::asrModelAt(int index) const {
    return m_catalog.asrModelAt(index);
}

ModelDescriptor ModelManager::translationModelAt(int index) const {
    return m_catalog.translationModelAt(index);
}

ModelDescriptor ModelManager::asrModelByUrl(const QString &url) const {
    return m_catalog.asrModelByUrl(url);
}

ModelDescriptor ModelManager::translationModelByUrl(const QString &url) const {
    return m_catalog.translationModelByUrl(url);
}
