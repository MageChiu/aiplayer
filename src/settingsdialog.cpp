#include "settingsdialog.h"

#include "libs/model/include/modeldownloadservice.h"
#include "shared/models/translation/translationsettings.h"
#include "platform/desktop/desktopmodelcoordinator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QCheckBox>
#include <QSettings>
#include <QLineEdit>
#include <QMessageBox>
#include <QtGlobal>

SettingsDialog::SettingsDialog(DesktopModelCoordinator *desktopModelCoordinator, QWidget *parent)
    : QDialog(parent), m_desktopModelCoordinator(desktopModelCoordinator) {
    Q_ASSERT(m_desktopModelCoordinator != nullptr);
    setWindowTitle(QStringLiteral("设置"));
    setMinimumWidth(400);

    auto *layout = new QVBoxLayout(this);

    // Whisper Model Group
    auto *modelLabel = new QLabel(QStringLiteral("Whisper 模型:"));
    m_modelCombo = new QComboBox();
    for (const ModelDescriptor &descriptor : m_desktopModelCoordinator->modelManager().asrModels()) {
        m_modelCombo->addItem(descriptor.displayName, descriptor.downloadUrl);
    }

    m_downloadButton = new QPushButton(QStringLiteral("下载模型"));
    connect(m_downloadButton, &QPushButton::clicked, this, &SettingsDialog::onAsrDownloadClicked);
    m_openModelDirButton = new QPushButton(QStringLiteral("打开模型目录"));
    connect(m_openModelDirButton, &QPushButton::clicked, this, &SettingsDialog::openAsrModelDirectory);

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

    auto *translationModeLabel = new QLabel(QStringLiteral("翻译模式:"));
    m_translationModeCombo = new QComboBox();
    m_translationModeCombo->addItem(QStringLiteral("在线翻译服务"), "online");
    m_translationModeCombo->addItem(QStringLiteral("本地 GGUF 模型 (llama.cpp)"), "local_gguf");
    auto *translationModeLayout = new QHBoxLayout();
    translationModeLayout->addWidget(translationModeLabel);
    translationModeLayout->addWidget(m_translationModeCombo, 1);

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

    auto *translationModelLabel = new QLabel(QStringLiteral("翻译模型:"));
    m_translationModelCombo = new QComboBox();
    for (const ModelDescriptor &descriptor : m_desktopModelCoordinator->modelManager().translationModels()) {
        m_translationModelCombo->addItem(descriptor.displayName, descriptor.downloadUrl);
    }
    m_translationDownloadButton = new QPushButton(QStringLiteral("下载模型"));
    connect(m_translationDownloadButton, &QPushButton::clicked, this, &SettingsDialog::onTranslationDownloadClicked);
    m_openTranslationModelDirButton = new QPushButton(QStringLiteral("打开翻译模型目录"));
    connect(m_openTranslationModelDirButton, &QPushButton::clicked, this, &SettingsDialog::openTranslationModelDirectory);
    auto *translationModelLayout = new QHBoxLayout();
    translationModelLayout->addWidget(translationModelLabel);
    translationModelLayout->addWidget(m_translationModelCombo, 1);
    translationModelLayout->addWidget(m_translationDownloadButton);
    translationModelLayout->addWidget(m_openTranslationModelDirButton);

    m_translationProgressBar = new QProgressBar();
    m_translationProgressBar->setRange(0, 100);
    m_translationProgressBar->setValue(0);
    m_translationProgressBar->hide();
    m_translationModelStatusLabel = new QLabel(QStringLiteral("翻译模型状态: 未下载"));

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
    layout->addLayout(translationModeLayout);
    layout->addLayout(providerLayout);
    layout->addLayout(baseUrlLayout);
    layout->addLayout(endpointLayout);
    layout->addLayout(apiKeyLayout);
    layout->addLayout(translationModelLayout);
    layout->addWidget(m_translationProgressBar);
    layout->addWidget(m_translationModelStatusLabel);
    layout->addWidget(m_translationHintLabel);
    layout->addSpacing(20);
    layout->addLayout(buttonLayout);

    loadSettings();
    updateUIState();

    connect(m_modelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::updateUIState);
    connect(m_translationModelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::updateUIState);
    connect(m_enableTranslationCheck, &QCheckBox::toggled, this, &SettingsDialog::updateUIState);
    connect(m_translationProviderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onTranslationProviderChanged);
    connect(m_translationModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onTranslationModeChanged);
    connect(m_desktopModelCoordinator->downloadService(), &ModelDownloadService::downloadProgress, this, &SettingsDialog::onDownloadProgress);
    connect(m_desktopModelCoordinator->downloadService(), &ModelDownloadService::downloadFinished, this, &SettingsDialog::onDownloadFinished);
    connect(m_desktopModelCoordinator->downloadService(), &ModelDownloadService::downloadFailed, this, &SettingsDialog::onDownloadFailed);
    connect(m_desktopModelCoordinator->downloadService(), &ModelDownloadService::downloadCancelled, this, &SettingsDialog::onDownloadCancelled);
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::loadSettings() {
    QSettings settings("AIPlayer", "Settings");
    const TranslationSettings translationSettings = TranslationSettings::load(settings);
    int modelIndex = settings.value("model_index", 0).toInt();
    if (modelIndex >= 0 && modelIndex < m_modelCombo->count()) {
        m_modelCombo->setCurrentIndex(modelIndex);
    }

    QString sourceLang = settings.value("source_lang", "auto").toString();
    int sourceIndex = m_sourceLangCombo->findData(sourceLang);
    if (sourceIndex >= 0) {
        m_sourceLangCombo->setCurrentIndex(sourceIndex);
    }

    m_enableTranslationCheck->setChecked(translationSettings.translationEnabled);
    
    QString targetLang = translationSettings.targetLanguage;
    int langIndex = m_targetLangCombo->findData(targetLang);
    if (langIndex >= 0) {
        m_targetLangCombo->setCurrentIndex(langIndex);
    }

    const QString translationMode = translationSettings.translationMode;
    const int translationModeIndex = m_translationModeCombo->findData(translationMode);
    if (translationModeIndex >= 0) {
        m_translationModeCombo->setCurrentIndex(translationModeIndex);
    }

    const QString provider = translationSettings.provider;
    const int providerIndex = m_translationProviderCombo->findData(provider);
    if (providerIndex >= 0) {
        m_translationProviderCombo->setCurrentIndex(providerIndex);
    }
    applyTranslationPreset(m_translationProviderCombo->currentData().toString(), false);

    m_translationBaseUrlEdit->setText(translationSettings.baseUrl.isEmpty() ? m_translationBaseUrlEdit->text() : translationSettings.baseUrl);
    m_translationEndpointEdit->setText(translationSettings.endpoint.isEmpty() ? m_translationEndpointEdit->text() : translationSettings.endpoint);
    m_translationApiKeyEdit->setText(translationSettings.apiKey);
    const int translationModelIndex = translationSettings.localModelIndex;
    if (translationModelIndex >= 0 && translationModelIndex < m_translationModelCombo->count()) {
        m_translationModelCombo->setCurrentIndex(translationModelIndex);
    }

    applyTranslationModePreset(m_translationModeCombo->currentData().toString());
}

void SettingsDialog::saveSettings() {
    QSettings settings("AIPlayer", "Settings");
    settings.setValue("model_index", m_modelCombo->currentIndex());
    TranslationSettings translationSettings;
    translationSettings.sourceLanguage = m_sourceLangCombo->currentData().toString();
    translationSettings.translationEnabled = m_enableTranslationCheck->isChecked();
    translationSettings.targetLanguage = m_targetLangCombo->currentData().toString();
    translationSettings.translationMode = m_translationModeCombo->currentData().toString();
    translationSettings.provider = m_translationProviderCombo->currentData().toString();
    translationSettings.baseUrl = m_translationBaseUrlEdit->text().trimmed();
    translationSettings.endpoint = m_translationEndpointEdit->text().trimmed();
    translationSettings.apiKey = m_translationApiKeyEdit->text().trimmed();
    translationSettings.localModelIndex = m_translationModelCombo->currentIndex();
    translationSettings.localModelUrl = m_translationModelCombo->currentData().toString();
    translationSettings.localModelPath = getTranslationModelPath();
    translationSettings.save(settings);
    accept();
}

QString SettingsDialog::asrModelDirectory() const {
    return m_desktopModelCoordinator->directoryFor(ModelKind::Asr);
}

QString SettingsDialog::translationModelDirectory() const {
    return m_desktopModelCoordinator->directoryFor(ModelKind::Translation);
}

void SettingsDialog::updateUIState() {
    const ModelDescriptor asrDescriptor = m_desktopModelCoordinator->modelManager().asrModelByUrl(m_modelCombo->currentData().toString());
    if (m_desktopModelCoordinator->isInstalled(asrDescriptor)) {
        m_statusLabel->setText(QStringLiteral("状态: 已存在 (目录: %1)").arg(asrModelDirectory()));
        m_downloadButton->setText(QStringLiteral("重新下载"));
    } else {
        m_statusLabel->setText(QStringLiteral("状态: 未下载 (目录: %1)").arg(asrModelDirectory()));
        m_downloadButton->setText(QStringLiteral("下载模型"));
    }

    const ModelDescriptor translationDescriptor = m_desktopModelCoordinator->modelManager().translationModelByUrl(m_translationModelCombo->currentData().toString());
    if (m_desktopModelCoordinator->isInstalled(translationDescriptor)) {
        m_translationModelStatusLabel->setText(QStringLiteral("翻译模型状态: 已存在 (目录: %1)").arg(translationModelDirectory()));
        m_translationDownloadButton->setText(QStringLiteral("重新下载"));
    } else {
        m_translationModelStatusLabel->setText(QStringLiteral("翻译模型状态: 未下载 (目录: %1)").arg(translationModelDirectory()));
        m_translationDownloadButton->setText(QStringLiteral("下载模型"));
    }

    const bool translationEnabled = m_enableTranslationCheck->isChecked();
    m_modelCombo->setEnabled(!m_desktopModelCoordinator->downloadService()->isDownloading() || m_activeDownloadKind != DownloadKind::AsrModel);
    m_targetLangCombo->setEnabled(translationEnabled);
    m_translationModeCombo->setEnabled(translationEnabled);
    const bool onlineMode = translationEnabled && m_translationModeCombo->currentData().toString() == QStringLiteral("online");
    const bool localMode = translationEnabled && m_translationModeCombo->currentData().toString() == QStringLiteral("local_gguf");
    m_translationProviderCombo->setEnabled(onlineMode);
    m_translationBaseUrlEdit->setEnabled(onlineMode);
    m_translationEndpointEdit->setEnabled(onlineMode);
    m_translationApiKeyEdit->setEnabled(onlineMode);
    m_translationModelCombo->setEnabled(localMode && !m_desktopModelCoordinator->downloadService()->isDownloading());
    m_translationDownloadButton->setEnabled(localMode);
    m_openTranslationModelDirButton->setEnabled(localMode);
    m_translationProgressBar->setEnabled(localMode);
    m_translationModelStatusLabel->setEnabled(localMode);
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

void SettingsDialog::applyTranslationModePreset(const QString &mode) {
    if (mode == QStringLiteral("local_gguf")) {
        m_translationHintLabel->setText(QStringLiteral("本地离线翻译将直接通过内置 llama.cpp API 执行，支持常见 GGUF 指令模型，如 Qwen、Llama、Gemma。建议将模型下载到独立的翻译模型目录。"));
    } else {
        applyTranslationPreset(m_translationProviderCombo->currentData().toString(), false);
    }
}

void SettingsDialog::onTranslationModeChanged() {
    applyTranslationModePreset(m_translationModeCombo->currentData().toString());
    updateUIState();
}

void SettingsDialog::startModelDownload(const ModelDescriptor &descriptor, DownloadKind kind) {
    m_activeDownloadKind = kind;
    if (m_desktopModelCoordinator->downloadService()->isDownloading()) {
        m_desktopModelCoordinator->downloadService()->cancelDownload();
        return;
    }

    const DownloadTask task{
        descriptor.downloadUrl,
        m_desktopModelCoordinator->filePathFor(descriptor),
        descriptor.displayName
    };
    if (!m_desktopModelCoordinator->downloadService()->startDownload(task)) {
        QMessageBox::critical(this, QStringLiteral("错误"), QStringLiteral("无法启动模型下载：%1").arg(descriptor.displayName));
        return;
    }

    QComboBox *combo = kind == DownloadKind::AsrModel ? m_modelCombo : m_translationModelCombo;
    QProgressBar *progressBar = kind == DownloadKind::AsrModel ? m_progressBar : m_translationProgressBar;
    QPushButton *button = kind == DownloadKind::AsrModel ? m_downloadButton : m_translationDownloadButton;
    QLabel *statusLabel = kind == DownloadKind::AsrModel ? m_statusLabel : m_translationModelStatusLabel;
    button->setText(QStringLiteral("取消下载"));
    combo->setEnabled(false);
    progressBar->setValue(0);
    progressBar->show();
    statusLabel->setText(kind == DownloadKind::AsrModel ? QStringLiteral("状态: 下载中...") : QStringLiteral("翻译模型状态: 下载中..."));
}

void SettingsDialog::onAsrDownloadClicked() {
    startModelDownload(m_desktopModelCoordinator->modelManager().asrModelByUrl(m_modelCombo->currentData().toString()), DownloadKind::AsrModel);
}

void SettingsDialog::onTranslationDownloadClicked() {
    startModelDownload(m_desktopModelCoordinator->modelManager().translationModelByUrl(m_translationModelCombo->currentData().toString()), DownloadKind::TranslationModel);
}

void SettingsDialog::onDownloadProgress(const DownloadTask &, const DownloadProgress &progress) {
    QProgressBar *progressBar = m_activeDownloadKind == DownloadKind::TranslationModel ? m_translationProgressBar : m_progressBar;
    QLabel *statusLabel = m_activeDownloadKind == DownloadKind::TranslationModel ? m_translationModelStatusLabel : m_statusLabel;
    if (progress.bytesTotal > 0) {
        progressBar->setMaximum(100);
        progressBar->setValue(progress.percentage());
        statusLabel->setText((m_activeDownloadKind == DownloadKind::TranslationModel ? QStringLiteral("翻译模型状态: 下载中 %1%") : QStringLiteral("状态: 下载中 %1%")).arg(progress.percentage()));
    } else {
        progressBar->setMaximum(0);
        statusLabel->setText((m_activeDownloadKind == DownloadKind::TranslationModel ? QStringLiteral("翻译模型状态: 下载中... %1 KB") : QStringLiteral("状态: 下载中... %1 KB")).arg(progress.bytesReceived / 1024));
    }
}

void SettingsDialog::onDownloadFinished(const DownloadTask &) {
    if (m_activeDownloadKind == DownloadKind::TranslationModel) {
        m_translationModelStatusLabel->setText(QStringLiteral("翻译模型状态: 下载完成"));
    } else {
        m_statusLabel->setText(QStringLiteral("状态: 下载完成"));
    }
    QMessageBox::information(this, QStringLiteral("成功"), QStringLiteral("模型下载完成！"));
    m_activeDownloadKind = DownloadKind::None;
    m_downloadButton->setText(QStringLiteral("下载模型"));
    m_modelCombo->setEnabled(true);
    m_progressBar->hide();
    m_translationDownloadButton->setText(QStringLiteral("下载模型"));
    m_translationModelCombo->setEnabled(true);
    m_translationProgressBar->hide();
    updateUIState();
}

void SettingsDialog::onDownloadFailed(const DownloadTask &, const QString &message) {
    if (m_activeDownloadKind == DownloadKind::TranslationModel) {
        m_translationModelStatusLabel->setText(QStringLiteral("翻译模型状态: 下载失败"));
    } else {
        m_statusLabel->setText(QStringLiteral("状态: 下载失败"));
    }
    QMessageBox::critical(this, QStringLiteral("下载错误"), message);
    m_activeDownloadKind = DownloadKind::None;
    m_downloadButton->setText(QStringLiteral("下载模型"));
    m_modelCombo->setEnabled(true);
    m_progressBar->hide();
    m_translationDownloadButton->setText(QStringLiteral("下载模型"));
    m_translationModelCombo->setEnabled(true);
    m_translationProgressBar->hide();
    updateUIState();
}

void SettingsDialog::onDownloadCancelled(const DownloadTask &) {
    if (m_activeDownloadKind == DownloadKind::TranslationModel) {
        m_translationModelStatusLabel->setText(QStringLiteral("翻译模型状态: 下载已取消"));
    } else {
        m_statusLabel->setText(QStringLiteral("状态: 下载已取消"));
    }
    m_activeDownloadKind = DownloadKind::None;
    m_downloadButton->setText(QStringLiteral("下载模型"));
    m_modelCombo->setEnabled(true);
    m_progressBar->hide();
    m_translationDownloadButton->setText(QStringLiteral("下载模型"));
    m_translationModelCombo->setEnabled(true);
    m_translationProgressBar->hide();
    updateUIState();
}

void SettingsDialog::openAsrModelDirectory() {
    QString errorMessage;
    if (!m_desktopModelCoordinator->openDirectory(ModelKind::Asr, &errorMessage)) {
        QMessageBox::warning(this, QStringLiteral("打开目录失败"),
                             errorMessage);
    }
}

void SettingsDialog::openTranslationModelDirectory() {
    QString errorMessage;
    if (!m_desktopModelCoordinator->openDirectory(ModelKind::Translation, &errorMessage)) {
        QMessageBox::warning(this, QStringLiteral("打开目录失败"),
                             errorMessage);
    }
}

QString SettingsDialog::getAsrModelPath() const {
    return m_desktopModelCoordinator->filePathFor(
        m_desktopModelCoordinator->modelManager().asrModelByUrl(m_modelCombo->currentData().toString()));
}

QString SettingsDialog::getTranslationModelPath() const {
    return m_desktopModelCoordinator->filePathFor(
        m_desktopModelCoordinator->modelManager().translationModelByUrl(m_translationModelCombo->currentData().toString()));
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
