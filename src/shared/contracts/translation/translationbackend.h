#pragma once

#include "../../models/translation/translationsettings.h"

#include <QString>

#include <functional>

struct TranslationRequest {
    int index = -1;
    QString text;
    QString sourceLanguage;
    QString targetLanguage;
};

using TranslationSuccessCallback = std::function<void(int, const QString &)>;
using TranslationFailureCallback = std::function<void(int, const QString &)>;

class ITranslationBackend {
public:
    virtual ~ITranslationBackend() = default;

    virtual bool supports(const TranslationSettings &settings) const = 0;
    virtual void translate(const TranslationRequest &request,
                           const TranslationSettings &settings,
                           const TranslationSuccessCallback &onSuccess,
                           const TranslationFailureCallback &onFailure) = 0;
    virtual void cancelAll() = 0;
};
