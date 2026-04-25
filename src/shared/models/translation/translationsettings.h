#pragma once

#include <QtCore/QString>

class QSettings;

struct TranslationSettings {
    QString sourceLanguage = QStringLiteral("auto");
    QString targetLanguage = QStringLiteral("zh-CN");
    bool translationEnabled = false;
    QString translationMode = QStringLiteral("online");
    QString provider = QStringLiteral("google");
    QString baseUrl = QStringLiteral("https://translate.googleapis.com");
    QString endpoint = QStringLiteral("/translate_a/single?client=gtx&sl={sl}&tl={tl}&dt=t&q={q}");
    QString apiKey;
    int localModelIndex = 0;
    QString localModelUrl;
    QString localModelPath;

    static TranslationSettings load(QSettings &settings);
    void save(QSettings &settings) const;
};
