#pragma once

#include "../../contracts/model/modelcontracts.h"

class ISharedModelFacade : public ISharedModelCatalog {
public:
    virtual ~ISharedModelFacade() = default;

    virtual bool hasInstalledAsrModel() const = 0;
    virtual bool hasInstalledTranslationModel() const = 0;
};
