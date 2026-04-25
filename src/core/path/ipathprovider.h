#pragma once

#include <QtCore/QString>

class IPathProvider {
public:
    virtual ~IPathProvider() = default;

    virtual QString appDataDirectory() const = 0;
    virtual QString modelsDirectory() const = 0;
    virtual QString asrModelsDirectory() const = 0;
    virtual QString translationModelsDirectory() const = 0;
    virtual QString torrentsDirectory() const = 0;
};
