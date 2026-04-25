#pragma once

#include "../../contracts/translation/translationbackend.h"
#include "../../models/state/appstate.h"

class ISharedTranslationFacade {
public:
    virtual ~ISharedTranslationFacade() = default;

    virtual TranslationSettings settings() const = 0;
    virtual bool isTranslationEnabled() const = 0;
    virtual TranslationStatus translationStatus() const = 0;
    virtual const ITranslationBackend *activeBackend() const = 0;
};
