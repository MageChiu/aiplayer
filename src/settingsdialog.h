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

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;

    QString getModelPath() const;
    bool isTranslationEnabled() const;
    QString getTargetLanguage() const;

private slots:
    void onDownloadClicked();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onDownloadError(QNetworkReply::NetworkError code);
    void saveSettings();

private:
    void loadSettings();
    void updateUIState();

    QComboBox *m_modelCombo = nullptr;
    QPushButton *m_downloadButton = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QLabel *m_statusLabel = nullptr;

    QCheckBox *m_enableTranslationCheck = nullptr;
    QComboBox *m_targetLangCombo = nullptr;
    
    QPushButton *m_okButton = nullptr;
    QPushButton *m_cancelButton = nullptr;

    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_currentReply = nullptr;
    QFile *m_outputFile = nullptr;
};