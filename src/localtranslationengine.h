#pragma once

#include <QString>

class LocalTranslationEngine {
public:
    LocalTranslationEngine();
    ~LocalTranslationEngine();

    bool ensureModelLoaded(const QString &modelPath, QString *errorMessage);
    QString translate(const QString &prompt, int maxTokens, QString *errorMessage);

private:
    QString m_modelPath;
    QString resolveHelperPath() const;
};
