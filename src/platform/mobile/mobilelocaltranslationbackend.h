#pragma once

#include "shared/contracts/translation/translationbackend.h"

class MobileLocalTranslationBackend : public ITranslationBackend {
public:
    bool supports(const TranslationSettings &settings) const override {
        return settings.translationMode == QStringLiteral("local_gguf");
    }

    void translate(const TranslationRequest &request,
                   const TranslationSettings &,
                   const TranslationSuccessCallback &,
                   const TranslationFailureCallback &onFailure) override {
        if (onFailure) {
            onFailure(request.index, QStringLiteral("移动端本地翻译后端尚未实现"));
        }
    }

    void cancelAll() override {}
};
