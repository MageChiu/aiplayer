#pragma once

#include <QObject>

#include <memory>

class ITranslationBackend;

class TranslationService : public QObject {
    Q_OBJECT

public:
    explicit TranslationService(QObject *parent = nullptr);
    ~TranslationService() override;

    void requestTranslation(int index, const QString &text);
    void clearPending();

signals:
    void translationReady(int index, const QString &translatedText);
    void translationFailed(const QString &message);

private:
    ITranslationBackend *selectBackend(const QString &mode) const;

    std::unique_ptr<ITranslationBackend> m_onlineBackend;
    std::unique_ptr<ITranslationBackend> m_desktopLocalBackend;
    std::unique_ptr<ITranslationBackend> m_mobileLocalBackend;
};
