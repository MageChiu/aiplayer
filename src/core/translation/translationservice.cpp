#include "translationservice.h"

#include "libs/logging/include/logcenter.h"
#include "shared/contracts/translation/translationbackend.h"
#include "shared/models/translation/translationsettings.h"
#include "platform/desktop/desktoplocaltranslationbackend.h"
#include "platform/desktop/onlinetranslationbackend.h"
#include "platform/mobile/mobilelocaltranslationbackend.h"

#include <QSettings>

TranslationService::TranslationService(QObject *parent)
    : QObject(parent),
      m_onlineBackend(std::make_unique<OnlineTranslationBackend>(this)),
      m_desktopLocalBackend(std::make_unique<DesktopLocalTranslationBackend>(this)),
      m_mobileLocalBackend(std::make_unique<MobileLocalTranslationBackend>()) {
}

TranslationService::~TranslationService() {
    clearPending();
}

void TranslationService::clearPending() {
    m_onlineBackend->cancelAll();
    m_desktopLocalBackend->cancelAll();
    m_mobileLocalBackend->cancelAll();
}

void TranslationService::requestTranslation(int index, const QString &text) {
    QSettings settings(QStringLiteral("AIPlayer"), QStringLiteral("Settings"));
    const TranslationSettings translationSettings = TranslationSettings::load(settings);
    if (!translationSettings.translationEnabled) {
        return;
    }

    ITranslationBackend *backend = selectBackend(translationSettings.translationMode);
    if (!backend) {
        emit translationFailed(QStringLiteral("未找到可用翻译后端：%1").arg(translationSettings.translationMode));
        return;
    }

    const TranslationRequest request{
        index,
        text,
        translationSettings.sourceLanguage,
        translationSettings.targetLanguage
    };
    backend->translate(
        request,
        translationSettings,
        [this](int translatedIndex, const QString &translatedText) {
            emit translationReady(translatedIndex, translatedText);
        },
        [this](int, const QString &message) {
            LogCenter::instance().appendLog(QStringLiteral("translation"), message);
            emit translationFailed(message);
        });
}

ITranslationBackend *TranslationService::selectBackend(const QString &mode) const {
    if (mode == QStringLiteral("local_gguf")) {
        return m_desktopLocalBackend.get();
    }
    return m_onlineBackend.get();
}
