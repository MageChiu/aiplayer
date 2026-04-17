#pragma once

#include <QMainWindow>

class QLabel;
class QPushButton;
class QSlider;
class QComboBox;
class MpvWidget;
class QWidget;
class QKeyEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

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

private:
    void tryAutoLoadFromArgs();
    QString formatTime(double seconds);

    MpvWidget *m_playerWidget = nullptr;
    QPushButton *m_playButton = nullptr;
    QPushButton *m_pauseButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    
    QSlider *m_seekSlider = nullptr;
    QLabel *m_timeLabel = nullptr;
    QComboBox *m_speedComboBox = nullptr;
    QPushButton *m_settingsButton = nullptr;
    QPushButton *m_helpButton = nullptr;

    QPushButton *m_muteButton = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QPushButton *m_fullscreenButton = nullptr;

    bool m_isSeeking = false;
    bool m_isMuted = false;
    double m_duration = 0.0;

    // Phase 2/3：字幕 Overlay
    QWidget *m_subtitleOverlay = nullptr;
    QLabel *m_subtitleLabel = nullptr;
};
