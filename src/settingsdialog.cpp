#include "settingsdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QCheckBox>
#include <QSettings>
#include <QNetworkRequest>
#include <QUrl>
#include <QFileInfo>
#include <QLineEdit>
#include <QDir>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QMessageBox>
#include <QStandardPaths>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), m_networkManager(new QNetworkAccessManager(this)) {
    setWindowTitle(QStringLiteral("设置"));
    setMinimumWidth(400);

    auto *layout = new QVBoxLayout(this);

    // Whisper Model Group
    auto *modelLabel = new QLabel(QStringLiteral("Whisper 模型:"));
    m_modelCombo = new QComboBox();
    m_modelCombo->addItem(QStringLiteral("Tiny (ggml-tiny.bin)"), "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.bin");
    m_modelCombo->addItem(QStringLiteral("Base (ggml-base.bin)"), "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin");
    m_modelCombo->addItem(QStringLiteral("Small (ggml-small.bin)"), "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin");
    m_modelCombo->addItem(QStringLiteral("Medium (ggml-medium.bin)"), "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-medium.bin");
    m_modelCombo->addItem(QStringLiteral("Large V3 (ggml-large-v3.bin)"), "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-large-v3.bin");

    m_downloadButton = new QPushButton(QStringLiteral("下载模型"));
    connect(m_downloadButton, &QPushButton::clicked, this, &SettingsDialog::onDownloadClicked);
    m_openModelDirButton = new QPushButton(QStringLiteral("打开模型目录"));
    connect(m_openModelDirButton, &QPushButton::clicked, this, &SettingsDialog::openModelDirectory);

    auto *modelLayout = new QHBoxLayout();
    modelLayout->addWidget(modelLabel);
    modelLayout->addWidget(m_modelCombo, 1);
    modelLayout->addWidget(m_downloadButton);
    modelLayout->addWidget(m_openModelDirButton);

    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->hide();

    m_statusLabel = new QLabel(QStringLiteral("状态: 未下载"));

    // Translation Group
    auto *sourceLangLabel = new QLabel(QStringLiteral("原始语言:"));
    m_sourceLangCombo = new QComboBox();
    m_sourceLangCombo->addItem(QStringLiteral("自动检测 (Auto)"), "auto");
    m_sourceLangCombo->addItem(QStringLiteral("英语 (English)"), "en");
    m_sourceLangCombo->addItem(QStringLiteral("中文 (Chinese)"), "zh");
    m_sourceLangCombo->addItem(QStringLiteral("日语 (Japanese)"), "ja");
    m_sourceLangCombo->addItem(QStringLiteral("韩语 (Korean)"), "ko");
    m_sourceLangCombo->addItem(QStringLiteral("法语 (French)"), "fr");
    m_sourceLangCombo->addItem(QStringLiteral("德语 (German)"), "de");
    m_sourceLangCombo->addItem(QStringLiteral("西班牙语 (Spanish)"), "es");
    m_sourceLangCombo->addItem(QStringLiteral("俄语 (Russian)"), "ru");

    auto *sourceLayout = new QHBoxLayout();
    sourceLayout->addWidget(sourceLangLabel);
    sourceLayout->addWidget(m_sourceLangCombo);
    sourceLayout->addStretch();

    m_enableTranslationCheck = new QCheckBox(QStringLiteral("启用字幕翻译"));
    auto *targetLangLabel = new QLabel(QStringLiteral("目标语言:"));
    m_targetLangCombo = new QComboBox();
    m_targetLangCombo->addItem(QStringLiteral("中文 (Chinese)"), "zh-CN");
    m_targetLangCombo->addItem(QStringLiteral("英语 (English)"), "en");
    m_targetLangCombo->addItem(QStringLiteral("日语 (Japanese)"), "ja");
    m_targetLangCombo->addItem(QStringLiteral("韩语 (Korean)"), "ko");
    m_targetLangCombo->addItem(QStringLiteral("法语 (French)"), "fr");
    m_targetLangCombo->addItem(QStringLiteral("德语 (German)"), "de");
    m_targetLangCombo->addItem(QStringLiteral("西班牙语 (Spanish)"), "es");
    m_targetLangCombo->addItem(QStringLiteral("俄语 (Russian)"), "ru");

    auto *transLayout = new QHBoxLayout();
    transLayout->addWidget(m_enableTranslationCheck);
    transLayout->addStretch();
    transLayout->addWidget(targetLangLabel);
    transLayout->addWidget(m_targetLangCombo);

    auto *providerLabel = new QLabel(QStringLiteral("翻译服务:"));
    m_translationProviderCombo = new QComboBox();
    m_translationProviderCombo->addItem(QStringLiteral("Google Translate"), "google");
    m_translationProviderCombo->addItem(QStringLiteral("Google 兼容接口"), "google-compatible");
    m_translationProviderCombo->addItem(QStringLiteral("LibreTranslate"), "libretranslate");

    auto *providerLayout = new QHBoxLayout();
    providerLayout->addWidget(providerLabel);
    providerLayout->addWidget(m_translationProviderCombo, 1);

    auto *baseUrlLabel = new QLabel(QStringLiteral("基础地址:"));
    m_translationBaseUrlEdit = new QLineEdit();
    m_translationBaseUrlEdit->setPlaceholderText(QStringLiteral("例如: https://translate.googleapis.com"));
    auto *baseUrlLayout = new QHBoxLayout();
    baseUrlLayout->addWidget(baseUrlLabel);
    baseUrlLayout->addWidget(m_translationBaseUrlEdit, 1);

    auto *endpointLabel = new QLabel(QStringLiteral("接口入口:"));
    m_translationEndpointEdit = new QLineEdit();
    m_translationEndpointEdit->setPlaceholderText(QStringLiteral("例如: /translate_a/single?client=gtx&sl={sl}&tl={tl}&dt=t&q={q}"));
    auto *endpointLayout = new QHBoxLayout();
    endpointLayout->addWidget(endpointLabel);
    endpointLayout->addWidget(m_translationEndpointEdit, 1);

    auto *apiKeyLabel = new QLabel(QStringLiteral("API Key:"));
    m_translationApiKeyEdit = new QLineEdit();
    m_translationApiKeyEdit->setPlaceholderText(QStringLiteral("可选；LibreTranslate 等服务可使用"));
    auto *apiKeyLayout = new QHBoxLayout();
    apiKeyLayout->addWidget(apiKeyLabel);
    apiKeyLayout->addWidget(m_translationApiKeyEdit, 1);

    m_translationHintLabel = new QLabel();
    m_translationHintLabel->setWordWrap(true);
    m_translationHintLabel->setStyleSheet(QStringLiteral("color: #666;"));

    // Buttons
    auto *buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton(QStringLiteral("确定"));
    m_cancelButton = new QPushButton(QStringLiteral("取消"));
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);

    layout->addLayout(modelLayout);
    layout->addWidget(m_progressBar);
    layout->addWidget(m_statusLabel);
    layout->addSpacing(10);
    layout->addWidget(new QLabel(QStringLiteral("<b>语音及翻译设置</b>")));
    layout->addLayout(sourceLayout);
    layout->addLayout(transLayout);
    layout->addLayout(providerLayout);
    layout->addLayout(baseUrlLayout);
    layout->addLayout(endpointLayout);
    layout->addLayout(apiKeyLayout);
    layout->addWidget(m_translationHintLabel);
    layout->addSpacing(20);
    layout->addLayout(buttonLayout);

    loadSettings();
    updateUIState();

    connect(m_modelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::updateUIState);
    connect(m_enableTranslationCheck, &QCheckBox::toggled, this, &SettingsDialog::updateUIState);
    connect(m_translationProviderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onTranslationProviderChanged);
}

SettingsDialog::~SettingsDialog() {
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
    if (m_outputFile) {
        m_outputFile->close();
        delete m_outputFile;
    }
}

void SettingsDialog::loadSettings() {
    QSettings settings("AIPlayer", "Settings");
    int modelIndex = settings.value("model_index", 0).toInt();
    if (modelIndex >= 0 && modelIndex < m_modelCombo->count()) {
        m_modelCombo->setCurrentIndex(modelIndex);
    }

    QString sourceLang = settings.value("source_lang", "auto").toString();
    int sourceIndex = m_sourceLangCombo->findData(sourceLang);
    if (sourceIndex >= 0) {
        m_sourceLangCombo->setCurrentIndex(sourceIndex);
    }

    m_enableTranslationCheck->setChecked(settings.value("translation_enabled", false).toBool());
    
    QString targetLang = settings.value("target_lang", "zh-CN").toString();
    int langIndex = m_targetLangCombo->findData(targetLang);
    if (langIndex >= 0) {
        m_targetLangCombo->setCurrentIndex(langIndex);
    }

    const QString provider = settings.value("translation_provider", "google").toString();
    const int providerIndex = m_translationProviderCombo->findData(provider);
    if (providerIndex >= 0) {
        m_translationProviderCombo->setCurrentIndex(providerIndex);
    }
    applyTranslationPreset(m_translationProviderCombo->currentData().toString(), false);

    m_translationBaseUrlEdit->setText(settings.value("translation_base_url", m_translationBaseUrlEdit->text()).toString());
    m_translationEndpointEdit->setText(settings.value("translation_endpoint", m_translationEndpointEdit->text()).toString());
    m_translationApiKeyEdit->setText(settings.value("translation_api_key", "").toString());
}

void SettingsDialog::saveSettings() {
    QSettings settings("AIPlayer", "Settings");
    settings.setValue("model_index", m_modelCombo->currentIndex());
    settings.setValue("source_lang", m_sourceLangCombo->currentData().toString());
    settings.setValue("translation_enabled", m_enableTranslationCheck->isChecked());
    settings.setValue("target_lang", m_targetLangCombo->currentData().toString());
    settings.setValue("translation_provider", m_translationProviderCombo->currentData().toString());
    settings.setValue("translation_base_url", m_translationBaseUrlEdit->text().trimmed());
    settings.setValue("translation_endpoint", m_translationEndpointEdit->text().trimmed());
    settings.setValue("translation_api_key", m_translationApiKeyEdit->text().trimmed());
    accept();
}

QString SettingsDialog::modelDirectory() const {
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.isEmpty()) {
        return QDir::homePath() + QStringLiteral("/.aiplayer/models");
    }
    return QDir(baseDir).absoluteFilePath(QStringLiteral("models"));
}

void SettingsDialog::updateUIState() {
    QString urlStr = m_modelCombo->currentData().toString();
    QString fileName = QUrl(urlStr).fileName();
    QString filePath = QDir(modelDirectory()).absoluteFilePath(fileName);

    if (QFile::exists(filePath)) {
        m_statusLabel->setText(QStringLiteral("状态: 已存在 (目录: %1)").arg(modelDirectory()));
        m_downloadButton->setText(QStringLiteral("重新下载"));
    } else {
        m_statusLabel->setText(QStringLiteral("状态: 未下载 (目录: %1)").arg(modelDirectory()));
        m_downloadButton->setText(QStringLiteral("下载模型"));
    }

    const bool translationEnabled = m_enableTranslationCheck->isChecked();
    m_targetLangCombo->setEnabled(translationEnabled);
    m_translationProviderCombo->setEnabled(translationEnabled);
    m_translationBaseUrlEdit->setEnabled(translationEnabled);
    m_translationEndpointEdit->setEnabled(translationEnabled);
    m_translationApiKeyEdit->setEnabled(translationEnabled);
    m_translationHintLabel->setEnabled(translationEnabled);
}

void SettingsDialog::applyTranslationPreset(const QString &provider, bool forceOverwrite) {
    QString defaultBaseUrl;
    QString defaultEndpoint;
    QString hintText;

    if (provider == "libretranslate") {
        defaultBaseUrl = QStringLiteral("https://libretranslate.com");
        defaultEndpoint = QStringLiteral("/translate");
        hintText = QStringLiteral("LibreTranslate 使用 POST JSON 请求，常见字段包括 q/source/target/format/api_key。");
    } else if (provider == "google-compatible") {
        defaultBaseUrl = QStringLiteral("https://translate.googleapis.com");
        defaultEndpoint = QStringLiteral("/translate_a/single?client=gtx&sl={sl}&tl={tl}&dt=t&q={q}");
        hintText = QStringLiteral("Google 兼容接口使用 GET 请求；可修改基础地址接入自建或兼容镜像。支持占位符 {sl} / {tl} / {q}。");
    } else {
        defaultBaseUrl = QStringLiteral("https://translate.googleapis.com");
        defaultEndpoint = QStringLiteral("/translate_a/single?client=gtx&sl={sl}&tl={tl}&dt=t&q={q}");
        hintText = QStringLiteral("Google Translate 默认入口，使用 GET 请求。支持占位符 {sl} / {tl} / {q}。");
    }

    if (forceOverwrite || m_translationBaseUrlEdit->text().trimmed().isEmpty()) {
        m_translationBaseUrlEdit->setText(defaultBaseUrl);
    }
    if (forceOverwrite || m_translationEndpointEdit->text().trimmed().isEmpty()) {
        m_translationEndpointEdit->setText(defaultEndpoint);
    }
    m_translationHintLabel->setText(hintText);
}

void SettingsDialog::onTranslationProviderChanged() {
    applyTranslationPreset(m_translationProviderCombo->currentData().toString(), true);
    updateUIState();
}

void SettingsDialog::onDownloadClicked() {
    if (m_currentReply) {
        m_currentReply->abort();
        return;
    }

    QString urlStr = m_modelCombo->currentData().toString();
    QUrl url(urlStr);
    QString fileName = url.fileName();
    QString dirPath = modelDirectory();
    QDir().mkpath(dirPath);
    QString filePath = QDir(dirPath).absoluteFilePath(fileName);

    m_outputFile = new QFile(filePath, this);
    if (!m_outputFile->open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("无法创建文件: %1").arg(filePath));
        delete m_outputFile;
        m_outputFile = nullptr;
        return;
    }

    QNetworkRequest request(url);
    // Allow redirects
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_currentReply = m_networkManager->get(request);

    connect(m_currentReply, &QNetworkReply::readyRead, this, [this]() {
        if (m_outputFile) m_outputFile->write(m_currentReply->readAll());
    });
    connect(m_currentReply, &QNetworkReply::downloadProgress, this, &SettingsDialog::onDownloadProgress);
    connect(m_currentReply, &QNetworkReply::finished, this, &SettingsDialog::onDownloadFinished);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(m_currentReply, &QNetworkReply::errorOccurred, this, &SettingsDialog::onDownloadError);
#else
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &SettingsDialog::onDownloadError);
#endif

    m_downloadButton->setText(QStringLiteral("取消下载"));
    m_modelCombo->setEnabled(false);
    m_progressBar->setValue(0);
    m_progressBar->show();
    m_statusLabel->setText(QStringLiteral("状态: 下载中..."));
}

void SettingsDialog::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        m_progressBar->setMaximum(100);
        m_progressBar->setValue(static_cast<int>((bytesReceived * 100) / bytesTotal));
        m_statusLabel->setText(QStringLiteral("状态: 下载中 %1%").arg((bytesReceived * 100) / bytesTotal));
    } else {
        m_progressBar->setMaximum(0);
        m_statusLabel->setText(QStringLiteral("状态: 下载中... %1 KB").arg(bytesReceived / 1024));
    }
}

void SettingsDialog::onDownloadFinished() {
    if (!m_currentReply) return;

    if (m_outputFile) {
        m_outputFile->write(m_currentReply->readAll());
        m_outputFile->close();
        delete m_outputFile;
        m_outputFile = nullptr;
    }

    if (m_currentReply->error() == QNetworkReply::NoError) {
        m_statusLabel->setText(QStringLiteral("状态: 下载完成"));
        QMessageBox::information(this, QStringLiteral("成功"), QStringLiteral("模型下载完成！"));
    } else {
        m_statusLabel->setText(QStringLiteral("状态: 下载失败/取消"));
        QString filePath = QDir(modelDirectory()).absoluteFilePath(QUrl(m_modelCombo->currentData().toString()).fileName());
        QFile::remove(filePath); // Remove partial file
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    m_downloadButton->setText(QStringLiteral("下载模型"));
    m_modelCombo->setEnabled(true);
    m_progressBar->hide();
    updateUIState();
}

void SettingsDialog::onDownloadError(QNetworkReply::NetworkError code) {
    if (code == QNetworkReply::OperationCanceledError) {
        return; // Handled in finished
    }
    QMessageBox::critical(this, QStringLiteral("下载错误"), m_currentReply->errorString());
}

void SettingsDialog::openModelDirectory() {
    const QString dirPath = modelDirectory();
    QDir().mkpath(dirPath);
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath))) {
        QMessageBox::warning(this, QStringLiteral("打开目录失败"),
                             QStringLiteral("无法打开模型目录：%1").arg(dirPath));
    }
}

QString SettingsDialog::getModelPath() const {
    QString urlStr = m_modelCombo->currentData().toString();
    QString fileName = QUrl(urlStr).fileName();
    return QDir(modelDirectory()).absoluteFilePath(fileName);
}

QString SettingsDialog::getSourceLanguage() const {
    return m_sourceLangCombo->currentData().toString();
}

bool SettingsDialog::isTranslationEnabled() const {
    return m_enableTranslationCheck->isChecked();
}

QString SettingsDialog::getTargetLanguage() const {
    return m_targetLangCombo->currentData().toString();
}
