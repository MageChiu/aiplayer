#pragma once

#include "shared/contracts/translation/translationbackend.h"

#include <QObject>

class QNetworkAccessManager;

class OnlineTranslationBackend : public QObject, public ITranslationBackend {
    Q_OBJECT

public:
    explicit OnlineTranslationBackend(QObject *parent = nullptr);

    bool supports(const TranslationSettings &settings) const override;
    void translate(const TranslationRequest &request,
                   const TranslationSettings &settings,
                   const TranslationSuccessCallback &onSuccess,
                   const TranslationFailureCallback &onFailure) override;
    void cancelAll() override;

private:
    QNetworkAccessManager *m_networkManager = nullptr;
};
