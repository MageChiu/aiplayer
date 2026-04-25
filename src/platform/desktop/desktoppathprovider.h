#pragma once

#include "core/path/ipathprovider.h"

class DesktopPathProvider : public IPathProvider {
public:
    QString appDataDirectory() const override;
    QString modelsDirectory() const override;
    QString asrModelsDirectory() const override;
    QString translationModelsDirectory() const override;
    QString torrentsDirectory() const override;
};
