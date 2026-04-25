#pragma once

#include <QString>
#include <memory>

class IPathProvider;

namespace AppPaths {

void setPathProvider(std::shared_ptr<const IPathProvider> provider);
const IPathProvider &pathProvider();

QString appDataDirectory();
QString modelsDirectory();
QString asrModelsDirectory();
QString translationModelsDirectory();
QString torrentsDirectory();

QString modelFilePathForUrl(const QString &directory, const QString &url);
QString asrModelFilePathForUrl(const QString &url);
QString translationModelFilePathForUrl(const QString &url);

}
