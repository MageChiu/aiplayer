#pragma once

#include <QMainWindow>

class QLabel;
class QPushButton;
class MpvWidget;
class QWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void openFile();
    void play();
    void pause();
    void updatePlaybackState(bool paused);
    void updateLoadedFile(const QString &filePath);
    void showError(const QString &message);

private:
    MpvWidget *m_playerWidget = nullptr;
    QPushButton *m_playButton = nullptr;
    QPushButton *m_pauseButton = nullptr;
    QLabel *m_statusLabel = nullptr;

    // Phase 2/3：字幕 Overlay
    QWidget *m_subtitleOverlay = nullptr;
    QLabel *m_subtitleLabel = nullptr;
};
