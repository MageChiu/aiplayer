#include "mainwindow.h"

#include "mpvwidget.h"

#include "settingsdialog.h"
#include "helpdialog.h"
#include "logcenter.h"
#include "logwindow.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QKeyEvent>

#include <QInputDialog>

#include <QGraphicsDropShadowEffect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>

namespace {
void reportOpenVideoCrashDebug(const QString &hypothesisId, const QString &location, const QString &message, const QJsonObject &data = {}) {
    if (qEnvironmentVariableIntValue("AIPLAYER_DEBUG_RUNTIME") == 0) {
        return;
    }
    static auto *manager = new QNetworkAccessManager(qApp);
    QNetworkRequest request(QUrl(QStringLiteral("http://127.0.0.1:7777/event")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    const QJsonObject payload{
        {QStringLiteral("sessionId"), QStringLiteral("open-video-crash")},
        {QStringLiteral("runId"), QStringLiteral("pre")},
        {QStringLiteral("hypothesisId"), hypothesisId},
        {QStringLiteral("location"), location},
        {QStringLiteral("msg"), QStringLiteral("[DEBUG] %1").arg(message)},
        {QStringLiteral("data"), data},
        {QStringLiteral("ts"), QString::number(QDateTime::currentMSecsSinceEpoch())}
    };
    manager->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
}

void reportTranslationDisplayDebug(const QString &hypothesisId, const QString &location, const QString &message, const QJsonObject &data = {}) {
    if (qEnvironmentVariableIntValue("AIPLAYER_DEBUG_TRANSLATION") == 0) {
        return;
    }
    static auto *manager = new QNetworkAccessManager(qApp);
    QNetworkRequest request(QUrl(QStringLiteral("http://127.0.0.1:7777/event")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    const QJsonObject payload{
        {QStringLiteral("sessionId"), QStringLiteral("translation-display")},
        {QStringLiteral("runId"), QStringLiteral("pre")},
        {QStringLiteral("hypothesisId"), hypothesisId},
        {QStringLiteral("location"), location},
        {QStringLiteral("msg"), QStringLiteral("[DEBUG] %1").arg(message)},
        {QStringLiteral("data"), data}
    };
    manager->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("AIPlayer - Phase 1"));
    resize(1100, 720);

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    m_playerWidget = new MpvWidget(central);

    // 用容器承载播放器 + 字幕 overlay
    auto *videoContainer = new QWidget(central);
    videoContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *videoLayout = new QVBoxLayout(videoContainer);
    videoLayout->setContentsMargins(0, 0, 0, 0);
    videoLayout->setSpacing(0);
    videoLayout->addWidget(m_playerWidget, 1);

    // 字幕 overlay（透明，置底部）
    m_subtitleOverlay = new QWidget(videoContainer);
    m_subtitleOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_subtitleOverlay->setStyleSheet("background: transparent;");

    m_subtitleLabel = new QLabel(m_subtitleOverlay);
    m_subtitleLabel->setWordWrap(true);
    m_subtitleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_subtitleLabel->setText(QStringLiteral("字幕：等待 ASR..."));
    m_subtitleLabel->setStyleSheet(
        "color: white;"
        "font-size: 22px;"
        "font-weight: 600;"
        "background-color: rgba(0,0,0,140);"
        "border-radius: 10px;"
        "padding: 10px 14px;");

    auto *shadow = new QGraphicsDropShadowEffect(m_subtitleLabel);
    shadow->setBlurRadius(18);
    shadow->setOffset(0, 2);
    m_subtitleLabel->setGraphicsEffect(shadow);

    layout->addWidget(videoContainer, 1);

    auto *progressLayout = new QHBoxLayout();
    m_timeLabel = new QLabel(QStringLiteral("00:00 / 00:00"), central);
    m_seekSlider = new QSlider(Qt::Horizontal, central);
    m_seekSlider->setRange(0, 1000);
    progressLayout->addWidget(m_timeLabel);
    progressLayout->addWidget(m_seekSlider);
    layout->addLayout(progressLayout);

    auto *controls = new QHBoxLayout();
    controls->setSpacing(8);

    auto *openButton = new QPushButton(QStringLiteral("打开视频"), central);
    auto *openUrlButton = new QPushButton(QStringLiteral("打开网络流"), central);
    m_playButton = new QPushButton(QStringLiteral("播放"), central);
    m_pauseButton = new QPushButton(QStringLiteral("暂停"), central);
    auto *stopButton = new QPushButton(QStringLiteral("停止"), central);
    auto *replayButton = new QPushButton(QStringLiteral("重播"), central);
    
    m_speedComboBox = new QComboBox(central);
    m_speedComboBox->addItem("0.5x", 0.5);
    m_speedComboBox->addItem("1.0x", 1.0);
    m_speedComboBox->addItem("1.25x", 1.25);
    m_speedComboBox->addItem("1.5x", 1.5);
    m_speedComboBox->addItem("2.0x", 2.0);
    m_speedComboBox->setCurrentIndex(1); // Default 1.0x
    
    m_settingsButton = new QPushButton(QStringLiteral("⚙ 设置"), central);
    m_settingsButton->setFocusPolicy(Qt::NoFocus);

    m_helpButton = new QPushButton(QStringLiteral("❓ 帮助"), central);
    m_helpButton->setFocusPolicy(Qt::NoFocus);

    m_logButton = new QPushButton(QStringLiteral("日志窗口"), central);
    m_logButton->setFocusPolicy(Qt::NoFocus);

    m_muteButton = new QPushButton(QStringLiteral("🔊"), central);
    m_muteButton->setFixedWidth(40);
    m_muteButton->setFocusPolicy(Qt::NoFocus);

    m_volumeSlider = new QSlider(Qt::Horizontal, central);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(100);
    m_volumeSlider->setFixedWidth(100);
    m_volumeSlider->setFocusPolicy(Qt::NoFocus);

    m_fullscreenButton = new QPushButton(QStringLiteral("⛶ 全屏"), central);
    m_fullscreenButton->setFocusPolicy(Qt::NoFocus);

    m_statusLabel = new QLabel(QStringLiteral("未加载文件"), central);
    m_statusLabel->setMinimumWidth(320);

    controls->addWidget(openButton);
    controls->addWidget(openUrlButton);
    controls->addWidget(m_playButton);
    controls->addWidget(m_pauseButton);
    controls->addWidget(stopButton);
    controls->addWidget(replayButton);
    controls->addWidget(new QLabel(QStringLiteral("倍速:")));
    controls->addWidget(m_speedComboBox);
    
    controls->addStretch(1);
    
    controls->addWidget(m_muteButton);
    controls->addWidget(m_volumeSlider);
    controls->addWidget(m_fullscreenButton);
    controls->addWidget(m_settingsButton);
    controls->addWidget(m_logButton);
    controls->addWidget(m_helpButton);

    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(m_statusLabel);
    
    layout->addLayout(controls);
    layout->addLayout(bottomLayout);
    setCentralWidget(central);

    connect(openButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(openUrlButton, &QPushButton::clicked, this, &MainWindow::openUrl);
    connect(m_playButton, &QPushButton::clicked, this, &MainWindow::play);
    connect(m_pauseButton, &QPushButton::clicked, this, &MainWindow::pause);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stop);
    connect(replayButton, &QPushButton::clicked, this, &MainWindow::replay);
    connect(m_playerWidget, &MpvWidget::playbackStateChanged, this, &MainWindow::updatePlaybackState);
    connect(m_playerWidget, &MpvWidget::timePosChanged, this, &MainWindow::onTimePosChanged);
    connect(m_playerWidget, &MpvWidget::durationChanged, this, &MainWindow::onDurationChanged);
    connect(m_playerWidget, &MpvWidget::volumeChanged, this, &MainWindow::onVolumeChanged);
    connect(m_playerWidget, &MpvWidget::muteStateChanged, this, &MainWindow::onMuteStateChanged);
    connect(m_playerWidget, &MpvWidget::fileLoaded, this, &MainWindow::updateLoadedFile);
    connect(m_playerWidget, &MpvWidget::errorOccurred, this, &MainWindow::showError);
    connect(m_playerWidget, &MpvWidget::errorOccurred, this, [](const QString &message) {
        LogCenter::instance().appendLog(QStringLiteral("error"), message);
        LogCenter::instance().setStatus(QStringLiteral("最近错误"), message);
    });

    connect(m_seekSlider, &QSlider::sliderPressed, this, [this]() { m_isSeeking = true; });
    connect(m_seekSlider, &QSlider::sliderReleased, this, [this]() {
        if (m_duration > 0.0) {
            m_playerWidget->seek(m_seekSlider->value() / 1000.0 * m_duration);
        }
        m_isSeeking = false;
    });
    connect(m_seekSlider, &QSlider::sliderMoved, this, &MainWindow::onSeekSliderMoved);
    connect(m_speedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSpeedChanged);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);
    connect(m_logButton, &QPushButton::clicked, this, &MainWindow::openLogWindow);
    connect(m_helpButton, &QPushButton::clicked, this, &MainWindow::openHelp);
    connect(m_muteButton, &QPushButton::clicked, this, &MainWindow::toggleMute);
    connect(m_volumeSlider, &QSlider::valueChanged, m_playerWidget, &MpvWidget::setVolume);
    connect(m_fullscreenButton, &QPushButton::clicked, this, &MainWindow::toggleFullscreen);

    // Prevent focus stealing for spacebar playback toggling
    openButton->setFocusPolicy(Qt::NoFocus);
    openUrlButton->setFocusPolicy(Qt::NoFocus);
    m_playButton->setFocusPolicy(Qt::NoFocus);
    m_pauseButton->setFocusPolicy(Qt::NoFocus);
    stopButton->setFocusPolicy(Qt::NoFocus);
    replayButton->setFocusPolicy(Qt::NoFocus);
    m_seekSlider->setFocusPolicy(Qt::NoFocus);
    m_speedComboBox->setFocusPolicy(Qt::NoFocus);

    // 订阅 ASR 文本更新，更新字幕 overlay
    connect(m_playerWidget, &MpvWidget::asrTextUpdated, this, [this](const QString &original, const QString &translated) {
        LogCenter::instance().setStatus(QStringLiteral("当前原文"), original.left(120));
        LogCenter::instance().setStatus(QStringLiteral("当前译文"), translated.left(120));
        if (!m_subtitleLabel) {
            return;
        }
        
        QString html;
        if (!translated.isEmpty()) {
            html = QStringLiteral("<div style='text-align:center;'>"
                                  "<span style='color: white; font-size: 22px; font-weight: bold;'>%1</span>"
                                  "<br/>"
                                  "<span style='color: #ffffb4; font-size: 18px;'>%2</span>"
                                  "</div>").arg(original, translated);
        } else {
            html = QStringLiteral("<div style='text-align:center;'>"
                                  "<span style='color: white; font-size: 22px; font-weight: bold;'>%1</span>"
                                  "</div>").arg(original);
        }

        // #region debug-point D:ui-display
        reportTranslationDisplayDebug(QStringLiteral("D"), QStringLiteral("mainwindow.cpp:asrTextUpdated"), QStringLiteral("ui received subtitle update"), {
            {QStringLiteral("original"), original},
            {QStringLiteral("translated"), translated},
            {QStringLiteral("html"), html}
        });
        // #endregion
        
        m_subtitleLabel->setText(html);

        // 简单布局：始终放在底部居中
        if (m_subtitleOverlay && m_playerWidget) {
            const QRect videoRect = m_playerWidget->geometry();
            const int marginBottom = 32;
            const int overlayHeight = 90; // slightly taller to fit two lines
            m_subtitleOverlay->setGeometry(videoRect.adjusted(0, videoRect.height() - overlayHeight - marginBottom, 0, -marginBottom));

            m_subtitleLabel->setGeometry(10, 10, m_subtitleOverlay->width() - 20, m_subtitleOverlay->height() - 20);
        }
    });

    updatePlaybackState(true);
    QTimer::singleShot(0, this, &MainWindow::tryAutoLoadFromArgs);
}

void MainWindow::openFile() {
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("选择视频文件"),
        QString(),
        QStringLiteral("视频文件 (*.mp4 *.mkv *.avi *.mov *.m4v);;所有文件 (*)"));

    if (filePath.isEmpty()) {
        return;
    }

    // #region debug-point D:file-selected
    reportOpenVideoCrashDebug(QStringLiteral("D"), QStringLiteral("mainwindow.cpp:229"), QStringLiteral("user selected local file"), {
        {QStringLiteral("filePath"), filePath},
        {QStringLiteral("exists"), QFileInfo::exists(filePath)},
        {QStringLiteral("size"), QString::number(QFileInfo(filePath).size())}
    });
    // #endregion
    m_playerWidget->loadFile(filePath);
}

void MainWindow::openUrl() {
    bool ok;
    QString url = QInputDialog::getText(this, 
                                        QStringLiteral("打开网络流"),
                                        QStringLiteral("请输入视频链接 (支持 http, https, rtmp, rtsp, m3u8, magnet 等协议):"),
                                        QLineEdit::Normal,
                                        QString(),
                                        &ok);
    if (ok && !url.isEmpty()) {
        m_playerWidget->loadFile(url);
    }
}

void MainWindow::tryAutoLoadFromArgs() {
    const QStringList args = QCoreApplication::arguments();
    if (args.size() < 2) {
        return;
    }

    const QString filePath = QFileInfo(args.at(1)).absoluteFilePath();
    if (!QFileInfo::exists(filePath)) {
        showError(QStringLiteral("命令行指定的视频文件不存在：%1").arg(filePath));
        return;
    }

    m_playerWidget->loadFile(filePath);
}

void MainWindow::play() {
    m_playerWidget->play();
}

void MainWindow::pause() {
    m_playerWidget->pause();
}

void MainWindow::stop() {
    m_playerWidget->stop();
    m_seekSlider->setValue(0);
    m_timeLabel->setText(QStringLiteral("00:00 / %1").arg(formatTime(m_duration)));
}

void MainWindow::replay() {
    m_playerWidget->replay();
}

void MainWindow::updatePlaybackState(bool paused) {
    m_playButton->setEnabled(paused);
    m_pauseButton->setEnabled(!paused);
}

void MainWindow::updateLoadedFile(const QString &filePath) {
    m_statusLabel->setText(QStringLiteral("当前文件：%1").arg(filePath));
}

void MainWindow::showError(const QString &message) {
    QMessageBox::critical(this, QStringLiteral("AIPlayer 错误"), message);
}

QString MainWindow::formatTime(double seconds) {
    int h = static_cast<int>(seconds) / 3600;
    int m = (static_cast<int>(seconds) % 3600) / 60;
    int s = static_cast<int>(seconds) % 60;
    if (h > 0) {
        return QString::asprintf("%02d:%02d:%02d", h, m, s);
    }
    return QString::asprintf("%02d:%02d", m, s);
}

void MainWindow::onTimePosChanged(double pos) {
    if (m_isSeeking || m_duration <= 0.0) return;
    
    m_seekSlider->setValue(static_cast<int>(pos / m_duration * 1000));
    m_timeLabel->setText(QStringLiteral("%1 / %2").arg(formatTime(pos), formatTime(m_duration)));
}

void MainWindow::onDurationChanged(double duration) {
    m_duration = duration;
    m_timeLabel->setText(QStringLiteral("00:00 / %1").arg(formatTime(duration)));
}

void MainWindow::onSeekSliderMoved(int value) {
    if (m_duration <= 0.0) return;
    double pos = value / 1000.0 * m_duration;
    m_timeLabel->setText(QStringLiteral("%1 / %2").arg(formatTime(pos), formatTime(m_duration)));
}

void MainWindow::onSpeedChanged(int index) {
    double speed = m_speedComboBox->itemData(index).toDouble();
    m_playerWidget->setPlaybackSpeed(speed);
}

void MainWindow::toggleMute() {
    m_playerWidget->setMute(!m_isMuted);
}

void MainWindow::toggleFullscreen() {
    if (isFullScreen()) {
        showNormal();
        m_fullscreenButton->setText(QStringLiteral("⛶ 全屏"));
    } else {
        showFullScreen();
        m_fullscreenButton->setText(QStringLiteral("⛶ 退出"));
    }
}

void MainWindow::onVolumeChanged(int volume) {
    m_volumeSlider->blockSignals(true);
    m_volumeSlider->setValue(volume);
    m_volumeSlider->blockSignals(false);
}

void MainWindow::onMuteStateChanged(bool mute) {
    m_isMuted = mute;
    m_muteButton->setText(mute ? QStringLiteral("🔇") : QStringLiteral("🔊"));
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (!m_playerWidget) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Space) {
        m_playerWidget->togglePause();
    } else if (event->key() == Qt::Key_Left) {
        m_playerWidget->seekRelative(-5.0);
    } else if (event->key() == Qt::Key_Right) {
        m_playerWidget->seekRelative(5.0);
    } else if (event->key() == Qt::Key_Up) {
        int newVol = std::min(100, m_volumeSlider->value() + 5);
        m_playerWidget->setVolume(newVol);
    } else if (event->key() == Qt::Key_Down) {
        int newVol = std::max(0, m_volumeSlider->value() - 5);
        m_playerWidget->setVolume(newVol);
    } else if (event->key() == Qt::Key_F) {
        toggleFullscreen();
    } else if (event->key() == Qt::Key_Escape && isFullScreen()) {
        toggleFullscreen();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::openSettings() {
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Settings are saved via QSettings inside the dialog
        // Trigger re-translation for existing subtitles using the new config
        if (m_playerWidget) {
            m_playerWidget->reTranslateAll();
        }
    }
}

void MainWindow::openHelp() {
    HelpDialog dialog(this);
    dialog.exec();
}

void MainWindow::openLogWindow() {
    if (!m_logWindow) {
        m_logWindow = new LogWindow();
        m_logWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_logWindow, &QObject::destroyed, this, [this]() {
            m_logWindow = nullptr;
        });
    }
    m_logWindow->show();
    m_logWindow->raise();
    m_logWindow->activateWindow();
}
