#include "apppaths.h"

#include "core/path/ipathprovider.h"

#include <QDir>
#include <QStandardPaths>
#include <QUrl>

#include <memory>
#include <utility>

namespace AppPaths {

namespace {
class FallbackPathProvider : public IPathProvider {
public:
    QString appDataDirectory() const override {
        const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (baseDir.isEmpty()) {
            return QDir::homePath() + QStringLiteral("/.aiplayer");
        }
        return baseDir;
    }

    QString modelsDirectory() const override {
        return QDir(appDataDirectory()).absoluteFilePath(QStringLiteral("models"));
    }

    QString asrModelsDirectory() const override {
        return QDir(modelsDirectory()).absoluteFilePath(QStringLiteral("asr"));
    }

    QString translationModelsDirectory() const override {
        return QDir(modelsDirectory()).absoluteFilePath(QStringLiteral("translation"));
    }

    QString torrentsDirectory() const override {
        return QDir(appDataDirectory()).absoluteFilePath(QStringLiteral("torrents"));
    }
};

std::shared_ptr<const IPathProvider> &providerStorage() {
    static std::shared_ptr<const IPathProvider> provider;
    return provider;
}

const IPathProvider &fallbackProvider() {
    static const FallbackPathProvider provider;
    return provider;
}
}

void setPathProvider(std::shared_ptr<const IPathProvider> provider) {
    providerStorage() = std::move(provider);
}

const IPathProvider &pathProvider() {
    const auto &provider = providerStorage();
    return provider ? *provider : fallbackProvider();
}

QString appDataDirectory() {
    return pathProvider().appDataDirectory();
}

QString modelsDirectory() {
    return pathProvider().modelsDirectory();
}

QString asrModelsDirectory() {
    return pathProvider().asrModelsDirectory();
}

QString translationModelsDirectory() {
    return pathProvider().translationModelsDirectory();
}

QString torrentsDirectory() {
    return pathProvider().torrentsDirectory();
}

QString modelFilePathForUrl(const QString &directory, const QString &url) {
    return QDir(directory).absoluteFilePath(QUrl(url).fileName());
}

QString asrModelFilePathForUrl(const QString &url) {
    return modelFilePathForUrl(asrModelsDirectory(), url);
}

QString translationModelFilePathForUrl(const QString &url) {
    return modelFilePathForUrl(translationModelsDirectory(), url);
}

}
