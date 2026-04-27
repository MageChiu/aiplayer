#pragma once

#include "shared/models/state/appstate.h"
#include "shared/models/subtitle/subtitlesegment.h"

#include <QOpenGLWidget>
#include <mpv/client.h>
#include <mpv/render_gl.h>

#include <atomic>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <thread>

class QOpenGLContext;
class QTimer;
#if AIPLAYER_ENABLE_TORRENT
class TorrentSessionController;
#endif
class AsrService;
class TranslationService;

class MpvWidget : public QOpenGLWidget {
    Q_OBJECT

public:
    explicit MpvWidget(QWidget *parent = nullptr);
    ~MpvWidget() override;

    bool initializePlayer();
    void loadFile(const QString &filePath);
    void play();
    void pause();
    void stop();
    void replay();
    void togglePause();
    bool isPaused() const;
    void seek(double pos);
    void seekRelative(double pos);
    void setPlaybackSpeed(double speed);
    void setVolume(int volume);
    void setMute(bool mute);
    void reTranslateAll();

signals:
    void playbackStateChanged(bool paused);
    void timePosChanged(double pos);
    void durationChanged(double duration);
    void volumeChanged(int volume);
    void muteStateChanged(bool mute);
    void fileLoaded(const QString &filePath);
    void errorOccurred(const QString &message);
    // 实时字幕文本更新信号（Phase 2/3）
    void asrTextUpdated(const QString &original, const QString &translated);
    void segmentRecognized(int index, const QString &text);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    bool event(QEvent *event) override;

private:
    static void onMpvWakeup(void *ctx);
    static void onUpdate(void *ctx);
    static void *getProcAddress(void *ctx, const char *name);

    void createMpv();
    void initializeRenderContext();
    void handleMpvEvents();
    void requestUpdate();
    void renderFrame();
    void setPaused(bool paused);
    bool isEofReached() const;
    void appendMpvLog(const QString &message);
    QString mpvErrorString(int errorCode) const;
    void extractAudioWithFFmpeg(const QString &videoPath);
    void stopAudioExtraction();
    void stopTorrentStreaming();
    void emitCurrentSubtitleUpdate();
    void applyTranslationResult(int index, const QString &translatedText);
    void handleTranslationFailure(const QString &message);

    // --- mpv 播放相关 ---
    mpv_handle *m_mpv = nullptr;
    mpv_render_context *m_renderContext = nullptr;
    QTimer *m_eventTimer = nullptr;
    bool m_initialized = false;
    bool m_paused = true;
    std::mutex m_logMutex;

    // --- ASR / Whisper 相关 ---
    std::vector<SubtitleSegment> m_subtitles;
    
    std::thread m_asrThread;
    std::thread m_audioExtractThread;
    std::atomic<bool> m_asrRunning{false};
    std::atomic<bool> m_audioExtractCancel{false};
    QString m_wavPath;
    QString m_asrStatus;
    std::mutex m_subtitleMutex;
#if AIPLAYER_ENABLE_TORRENT
    std::unique_ptr<TorrentSessionController> m_torrentController;
#endif
    std::unique_ptr<AsrService> m_asrService;
    std::unique_ptr<TranslationService> m_translationService;
    PlaybackState m_playbackState;
    SubtitleState m_subtitleState;
    TranslationState m_translationState;

    void updateAsrStatus(const QString &status);
    void runWhisper();
    void translateSegment(int index, const QString &text);
};
