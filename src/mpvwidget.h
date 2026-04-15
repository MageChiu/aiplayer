#pragma once

#include <QOpenGLWidget>
#include <mpv/client.h>
#include <mpv/render_gl.h>

// whisper.cpp 头文件
#include "whisper.h"

#include <atomic>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>

class QOpenGLContext;
class QTimer;

class MpvWidget : public QOpenGLWidget {
    Q_OBJECT

public:
    explicit MpvWidget(QWidget *parent = nullptr);
    ~MpvWidget() override;

    bool initializePlayer();
    void loadFile(const QString &filePath);
    void play();
    void pause();
    void togglePause();
    bool isPaused() const;

signals:
    void playbackStateChanged(bool paused);
    void fileLoaded(const QString &filePath);
    void errorOccurred(const QString &message);
    // 实时字幕文本更新信号（Phase 2/3）
    void asrTextUpdated(const QString &text);

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
    void appendMpvLog(const QString &message);
    QString mpvErrorString(int errorCode) const;

    // --- mpv 播放相关 ---
    mpv_handle *m_mpv = nullptr;
    mpv_render_context *m_renderContext = nullptr;
    QTimer *m_eventTimer = nullptr;
    bool m_initialized = false;
    bool m_paused = true;
    std::mutex m_logMutex;

    // --- ASR / Whisper 相关 ---
    // 简化版：先用一个后台线程周期性产出 mock 文本，打通 UI 链路
    std::thread m_asrThread;
    std::atomic<bool> m_asrRunning{false};
};
