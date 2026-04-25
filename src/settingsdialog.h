#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QPushButton;
class QProgressBar;
class QCheckBox;
class QLabel;
class QLineEdit;
class QWidget;
class DesktopModelCoordinator;
class ModelDownloadService;
struct DownloadTask;
struct DownloadProgress;
struct ModelDescriptor;
enum class ModelKind;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(DesktopModelCoordinator *desktopModelCoordinator, QWidget *parent = nullptr);
    ~SettingsDialog() override;

    QString getAsrModelPath() const;
    QString getTranslationModelPath() const;
    QString getSourceLanguage() const;
    bool isTranslationEnabled() const;
    QString getTargetLanguage() const;

private slots:
    void onAsrDownloadClicked();
    void onTranslationDownloadClicked();
    void openAsrModelDirectory();
    void openTranslationModelDirectory();
    void onDownloadProgress(const DownloadTask &task, const DownloadProgress &progress);
    void onDownloadFinished(const DownloadTask &task);
    void onDownloadFailed(const DownloadTask &task, const QString &message);
    void onDownloadCancelled(const DownloadTask &task);
    void saveSettings();
    void onTranslationProviderChanged();
    void onTranslationModeChanged();

private:
    enum class DownloadKind {
        None,
        AsrModel,
        TranslationModel
    };

    QString asrModelDirectory() const;
    QString translationModelDirectory() const;
    void startModelDownload(const ModelDescriptor &descriptor, DownloadKind kind);
    void loadSettings();
    void updateUIState();
    void applyTranslationPreset(const QString &provider, bool forceOverwrite);
    void applyTranslationModePreset(const QString &mode);

    QComboBox *m_modelCombo = nullptr;
    QPushButton *m_downloadButton = nullptr;
    QPushButton *m_openModelDirButton = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QLabel *m_statusLabel = nullptr;

    QComboBox *m_sourceLangCombo = nullptr;
    QCheckBox *m_enableTranslationCheck = nullptr;
    QComboBox *m_targetLangCombo = nullptr;
    QComboBox *m_translationModeCombo = nullptr;
    QComboBox *m_translationProviderCombo = nullptr;
    QLineEdit *m_translationBaseUrlEdit = nullptr;
    QLineEdit *m_translationEndpointEdit = nullptr;
    QLineEdit *m_translationApiKeyEdit = nullptr;
    QComboBox *m_translationModelCombo = nullptr;
    QPushButton *m_translationDownloadButton = nullptr;
    QPushButton *m_openTranslationModelDirButton = nullptr;
    QProgressBar *m_translationProgressBar = nullptr;
    QLabel *m_translationModelStatusLabel = nullptr;
    QLabel *m_translationHintLabel = nullptr;
    
    QPushButton *m_okButton = nullptr;
    QPushButton *m_cancelButton = nullptr;

    DesktopModelCoordinator *m_desktopModelCoordinator = nullptr;
    DownloadKind m_activeDownloadKind = DownloadKind::None;
};
