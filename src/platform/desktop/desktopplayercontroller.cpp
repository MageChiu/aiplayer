#include "platform/desktop/desktopplayercontroller.h"

#include "mpvwidget.h"

DesktopPlayerController::DesktopPlayerController(QObject *parent)
    : IPlayerController(parent), m_playerWidget(new MpvWidget()) {
    connect(m_playerWidget, &MpvWidget::playbackStateChanged, this, [this](bool paused) {
        m_state.playback.paused = paused;
        emit playbackStateChanged(paused);
    });
    connect(m_playerWidget, &MpvWidget::timePosChanged, this, [this](double pos) {
        m_state.playback.positionSeconds = pos;
        emit timePosChanged(pos);
    });
    connect(m_playerWidget, &MpvWidget::durationChanged, this, [this](double duration) {
        m_state.playback.durationSeconds = duration;
        emit durationChanged(duration);
    });
    connect(m_playerWidget, &MpvWidget::volumeChanged, this, [this](int volume) {
        m_state.playback.volume = volume;
        emit volumeChanged(volume);
    });
    connect(m_playerWidget, &MpvWidget::muteStateChanged, this, [this](bool mute) {
        m_state.playback.muted = mute;
        emit muteStateChanged(mute);
    });
    connect(m_playerWidget, &MpvWidget::fileLoaded, this, [this](const QString &filePath) {
        m_state.playback.loadedSource = filePath;
        emit fileLoaded(filePath);
    });
    connect(m_playerWidget, &MpvWidget::errorOccurred, this, [this](const QString &message) {
        m_state.translation.status = TranslationStatus::Failed;
        m_state.translation.lastError = message;
        emit errorOccurred(message);
    });
    connect(m_playerWidget, &MpvWidget::asrTextUpdated, this, [this](const QString &original, const QString &translated) {
        m_state.subtitles.currentText = original;
        m_state.subtitles.currentTranslatedText = translated;
        if (translated.isEmpty()) {
            m_state.subtitles.statusMessage = original;
        }
        emit asrTextUpdated(original, translated);
    });
}

DesktopPlayerController::~DesktopPlayerController() = default;

QWidget *DesktopPlayerController::videoOutputWidget() const {
    return m_playerWidget;
}

PlayerState DesktopPlayerController::currentState() const {
    return m_state;
}

void DesktopPlayerController::loadFile(const QString &filePath) {
    m_playerWidget->loadFile(filePath);
}

void DesktopPlayerController::play() {
    m_playerWidget->play();
}

void DesktopPlayerController::pause() {
    m_playerWidget->pause();
}

void DesktopPlayerController::stop() {
    m_playerWidget->stop();
}

void DesktopPlayerController::replay() {
    m_playerWidget->replay();
}

void DesktopPlayerController::togglePause() {
    m_playerWidget->togglePause();
}

void DesktopPlayerController::seek(double pos) {
    m_playerWidget->seek(pos);
}

void DesktopPlayerController::seekRelative(double pos) {
    m_playerWidget->seekRelative(pos);
}

void DesktopPlayerController::setPlaybackSpeed(double speed) {
    m_playerWidget->setPlaybackSpeed(speed);
}

void DesktopPlayerController::setVolume(int volume) {
    m_playerWidget->setVolume(volume);
}

void DesktopPlayerController::setMute(bool mute) {
    m_playerWidget->setMute(mute);
}

void DesktopPlayerController::reTranslateAll() {
    m_playerWidget->reTranslateAll();
}
