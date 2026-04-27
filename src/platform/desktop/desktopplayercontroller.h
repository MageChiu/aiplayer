#pragma once

#include "core/player/iplayercontroller.h"

class MpvWidget;

class DesktopPlayerController : public IPlayerController {
    Q_OBJECT

public:
    explicit DesktopPlayerController(QObject *parent = nullptr);
    ~DesktopPlayerController() override;

    QWidget *videoOutputWidget() const override;
    PlayerState currentState() const override;

    void loadFile(const QString &filePath) override;
    void play() override;
    void pause() override;
    void stop() override;
    void replay() override;
    void togglePause() override;
    void seek(double pos) override;
    void seekRelative(double pos) override;
    void setPlaybackSpeed(double speed) override;
    void setVolume(int volume) override;
    void setMute(bool mute) override;
    void reTranslateAll() override;

private:
    MpvWidget *m_playerWidget = nullptr;
    PlayerState m_state;
};
