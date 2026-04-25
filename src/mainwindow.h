#pragma once

#include <QMainWindow>
#include <QString>

class QLabel;
class QPushButton;
class QSlider;
class QComboBox;
class IPlayerController;
class DesktopModelCoordinator;
class QWidget;
class QKeyEvent;
class LogWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(IPlayerController *playerController,
                        DesktopModelCoordinator *desktopModelCoordinator,
                        QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void openFile();
    void openUrl();
    void play();
    void pause();
    void stop();
    void replay();
    void updatePlaybackState(bool paused);
    void updateLoadedFile(const QString &filePath);
    void showError(const QString &message);
    void onTimePosChanged(double pos);
    void onDurationChanged(double duration);
    void onSeekSliderMoved(int value);
    void onSpeedChanged(int index);
    void openSettings();
    void toggleMute();
    void toggleFullscreen();
    void onVolumeChanged(int volume);
    void onMuteStateChanged(bool mute);
    void openHelp();
    void openLogWindow();

private:
    void tryAutoLoadFromArgs();
    QString formatTime(double seconds);

    IPlayerController *m_playerController = nullptr;
    DesktopModelCoordinator *m_desktopModelCoordinator = nullptr;
    QPushButton *m_playButton = nullptr;
    QPushButton *m_pauseButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    
    QSlider *m_seekSlider = nullptr;
    QLabel *m_timeLabel = nullptr;
    QComboBox *m_speedComboBox = nullptr;
    QPushButton *m_settingsButton = nullptr;
    QPushButton *m_helpButton = nullptr;
    QPushButton *m_logButton = nullptr;

    QPushButton *m_muteButton = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QPushButton *m_fullscreenButton = nullptr;

    bool m_isSeeking = false;
    bool m_isMuted = false;
    double m_duration = 0.0;

    // Phase 2/3：字幕 Overlay
    QWidget *m_subtitleOverlay = nullptr;
    QLabel *m_subtitleLabel = nullptr;
    LogWindow *m_logWindow = nullptr;
};
