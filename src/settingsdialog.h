#pragma once

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

class QComboBox;
class QPushButton;
class QProgressBar;
class QCheckBox;
class QLabel;
class QLineEdit;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;

    QString getModelPath() const;
    QString getSourceLanguage() const;
    bool isTranslationEnabled() const;
    QString getTargetLanguage() const;

private slots:
    void onDownloadClicked();
    void openModelDirectory();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onDownloadError(QNetworkReply::NetworkError code);
    void saveSettings();
    void onTranslationProviderChanged();

private:
    QString modelDirectory() const;
    void loadSettings();
    void updateUIState();
    void applyTranslationPreset(const QString &provider, bool forceOverwrite);

    QComboBox *m_modelCombo = nullptr;
    QPushButton *m_downloadButton = nullptr;
    QPushButton *m_openModelDirButton = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QLabel *m_statusLabel = nullptr;

    QComboBox *m_sourceLangCombo = nullptr;
    QCheckBox *m_enableTranslationCheck = nullptr;
    QComboBox *m_targetLangCombo = nullptr;
    QComboBox *m_translationProviderCombo = nullptr;
    QLineEdit *m_translationBaseUrlEdit = nullptr;
    QLineEdit *m_translationEndpointEdit = nullptr;
    QLineEdit *m_translationApiKeyEdit = nullptr;
    QLabel *m_translationHintLabel = nullptr;
    
    QPushButton *m_okButton = nullptr;
    QPushButton *m_cancelButton = nullptr;

    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_currentReply = nullptr;
    QFile *m_outputFile = nullptr;
};
