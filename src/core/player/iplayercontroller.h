#pragma once

#include "shared/models/state/appstate.h"

#include <QObject>

class QWidget;

struct PlayerState {
    PlaybackState playback;
    SubtitleState subtitles;
    TranslationState translation;
};

struct PlayerError {
    QString source;
    QString message;
};

class IPlayerController : public QObject {
    Q_OBJECT

public:
    explicit IPlayerController(QObject *parent = nullptr) : QObject(parent) {}
    ~IPlayerController() override = default;

    virtual QWidget *videoOutputWidget() const = 0;
    virtual PlayerState currentState() const = 0;

    virtual void loadFile(const QString &filePath) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void replay() = 0;
    virtual void togglePause() = 0;
    virtual void seek(double pos) = 0;
    virtual void seekRelative(double pos) = 0;
    virtual void setPlaybackSpeed(double speed) = 0;
    virtual void setVolume(int volume) = 0;
    virtual void setMute(bool mute) = 0;
    virtual void reTranslateAll() = 0;

signals:
    void playbackStateChanged(bool paused);
    void timePosChanged(double pos);
    void durationChanged(double duration);
    void volumeChanged(int volume);
    void muteStateChanged(bool mute);
    void fileLoaded(const QString &filePath);
    void errorOccurred(const QString &message);
    void asrTextUpdated(const QString &original, const QString &translated);
};
