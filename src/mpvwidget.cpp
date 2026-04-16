#include "mpvwidget.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QMetaObject>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QProcess>
#include <QSurfaceFormat>
#include <QTimer>

#include <chrono>
#include <clocale>
#include <cstdio>
#include <cstring>

#include <QStandardPaths>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>

namespace {
constexpr QEvent::Type kMpvUpdateEvent = static_cast<QEvent::Type>(QEvent::User + 1);
}

MpvWidget::MpvWidget(QWidget *parent)
    : QOpenGLWidget(parent), m_eventTimer(new QTimer(this)), m_networkManager(new QNetworkAccessManager(this)) {
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    setMinimumSize(640, 360);

    connect(m_eventTimer, &QTimer::timeout, this, &MpvWidget::handleMpvEvents);
    m_eventTimer->setInterval(16);

    connect(this, &MpvWidget::segmentRecognized, this, &MpvWidget::translateSegment, Qt::QueuedConnection);
}

MpvWidget::~MpvWidget() {
    m_asrRunning.store(false);
    if (m_asrThread.joinable()) {
        m_asrThread.join();
    }

    if (m_ffmpegProcess) {
        m_ffmpegProcess->kill();
        m_ffmpegProcess->waitForFinished(3000);
        m_ffmpegProcess = nullptr;
    }

    if (m_webtorrentProcess) {
        m_webtorrentProcess->kill();
        m_webtorrentProcess->waitForFinished();
    }

    if (m_renderContext) {
        mpv_render_context_free(m_renderContext);
        m_renderContext = nullptr;
    }
    if (m_mpv) {
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }
}

bool MpvWidget::initializePlayer() {
    if (m_initialized) {
        return true;
    }

    createMpv();
    if (!m_mpv) {
        return false;
    }

    m_eventTimer->start();
    m_initialized = true;
    return true;
}

void MpvWidget::loadFile(const QString &filePath) {
    if (!initializePlayer()) {
        emit errorOccurred(QStringLiteral("播放器初始化失败"));
        return;
    }

    if (m_webtorrentProcess) {
        m_webtorrentProcess->kill();
        m_webtorrentProcess->waitForFinished();
        m_webtorrentProcess->deleteLater();
        m_webtorrentProcess = nullptr;
    }

    bool isNetworkStream = filePath.startsWith("http://", Qt::CaseInsensitive) ||
                           filePath.startsWith("https://", Qt::CaseInsensitive) ||
                           filePath.startsWith("rtmp://", Qt::CaseInsensitive) ||
                           filePath.startsWith("rtsp://", Qt::CaseInsensitive);

    bool isMagnet = filePath.startsWith("magnet:", Qt::CaseInsensitive);

    if (isNetworkStream || isMagnet) {
        // Clear subtitles and state since we can't easily extract audio with ffmpeg synchronously from a live stream
        // without more complex piping. For now, disable ASR on live streams.
        {
            std::lock_guard<std::mutex> lock(m_subtitleMutex);
            m_subtitles.clear();
            m_asrStatus = QStringLiteral("[ASR] 网络流/磁力链接暂不支持自动字幕");
        }
        emit asrTextUpdated(m_asrStatus, "");
    } else {
        extractAudioWithFFmpeg(filePath);
    }

    if (isMagnet) {
        m_webtorrentProcess = new QProcess(this);
        connect(m_webtorrentProcess, &QProcess::readyReadStandardOutput, this, [this]() {
            QString output = QString::fromUtf8(m_webtorrentProcess->readAllStandardOutput());
            std::fprintf(stderr, "[WebTorrent] %s", output.toUtf8().constData());
            
            // Parse Server running at: http://localhost:8000/0
            QRegularExpression re("Server running at: (http://[a-zA-Z0-9_.:]+/[0-9]+)");
            QRegularExpressionMatch match = re.match(output);
            if (match.hasMatch()) {
                QString streamUrl = match.captured(1);
                std::fprintf(stderr, "[WebTorrent] Found stream URL: %s\n", streamUrl.toUtf8().constData());
                
                const QByteArray utf8 = streamUrl.toUtf8();
                const char *cmd[] = {"loadfile", utf8.constData(), nullptr};
                mpv_command(m_mpv, cmd);
                setPaused(false);
                emit fileLoaded(QStringLiteral("磁力链接下载中..."));
                
                // Disconnect to avoid loading multiple times if it prints again
                disconnect(m_webtorrentProcess, &QProcess::readyReadStandardOutput, this, nullptr);
            }
        });
        
        connect(m_webtorrentProcess, &QProcess::readyReadStandardError, this, [this]() {
            std::fprintf(stderr, "[WebTorrent ERR] %s", m_webtorrentProcess->readAllStandardError().constData());
        });
        
        connect(m_webtorrentProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus status) {
            std::fprintf(stderr, "[WebTorrent] Process exited with code %d\n", exitCode);
        });

        // Use webtorrent cli tool (needs to be installed globally via npm install -g webtorrent-cli)
        m_webtorrentProcess->start("webtorrent", QStringList() << filePath << "--keep-seeding");
        if (!m_webtorrentProcess->waitForStarted(3000)) {
            emit errorOccurred(QStringLiteral("无法启动 webtorrent 进程。请确保系统已安装 Node.js 并且通过 'npm install -g webtorrent-cli' 安装了依赖。"));
            m_webtorrentProcess->deleteLater();
            m_webtorrentProcess = nullptr;
        }
        return;
    }

    const QByteArray utf8 = filePath.toUtf8();
    const char *cmd[] = {"loadfile", utf8.constData(), nullptr};
    if (mpv_command(m_mpv, cmd) < 0) {
        emit errorOccurred(QStringLiteral("无法加载文件：%1").arg(filePath));
        return;
    }

    setPaused(false);
    emit fileLoaded(filePath);
}

void MpvWidget::play() {
    setPaused(false);
}

void MpvWidget::pause() {
    setPaused(true);
}

void MpvWidget::togglePause() {
    setPaused(!m_paused);
}

bool MpvWidget::isPaused() const {
    return m_paused;
}

void MpvWidget::initializeGL() {
    if (!initializePlayer()) {
        return;
    }

    initializeRenderContext();
}

void MpvWidget::paintGL() {
    if (!m_renderContext) {
        initializeRenderContext();
    }

    renderFrame();
}

void MpvWidget::resizeGL(int width, int height) {
    Q_UNUSED(width);
    Q_UNUSED(height);
    update();
}

bool MpvWidget::event(QEvent *event) {
    if (event->type() == kMpvUpdateEvent) {
        renderFrame();
        return true;
    }
    return QWidget::event(event);
}

void MpvWidget::onMpvWakeup(void *ctx) {
    auto *self = static_cast<MpvWidget *>(ctx);
    QMetaObject::invokeMethod(self, &MpvWidget::handleMpvEvents, Qt::QueuedConnection);
}

void MpvWidget::onUpdate(void *ctx) {
    auto *self = static_cast<MpvWidget *>(ctx);
    QMetaObject::invokeMethod(self, [self]() { self->update(); }, Qt::QueuedConnection);
}

void *MpvWidget::getProcAddress(void *ctx, const char *name) {
    auto *glContext = static_cast<QOpenGLContext *>(ctx);
    if (!glContext) {
        glContext = QOpenGLContext::currentContext();
    }
    if (!glContext) {
        return nullptr;
    }
    return reinterpret_cast<void *>(glContext->getProcAddress(name));
}

void MpvWidget::createMpv() {
    if (m_mpv) {
        return;
    }

    setlocale(LC_NUMERIC, "C");
    appendMpvLog(QStringLiteral("createMpv: begin"));
    m_mpv = mpv_create();
    if (!m_mpv) {
        appendMpvLog(QStringLiteral("createMpv: mpv_create() returned nullptr"));
        emit errorOccurred(QStringLiteral("无法创建 mpv 实例"));
        return;
    }

    appendMpvLog(QStringLiteral("createMpv: mpv_create() succeeded"));
    mpv_request_log_messages(m_mpv, "v");
    mpv_set_option_string(m_mpv, "terminal", "no");
    mpv_set_option_string(m_mpv, "msg-level", "all=v");
    mpv_set_option_string(m_mpv, "vo", "libmpv");
#if defined(Q_OS_MAC)
    mpv_set_option_string(m_mpv, "gpu-context", "cocoa");
#elif defined(Q_OS_WIN)
    mpv_set_option_string(m_mpv, "gpu-context", "d3d11");
#endif
    mpv_set_option_string(m_mpv, "profile", "sw-fast");
    mpv_set_option_string(m_mpv, "hwdec", "no");
    mpv_set_option_string(m_mpv, "vd-lavc-dr", "no");
    mpv_set_wakeup_callback(m_mpv, &MpvWidget::onMpvWakeup, this);

    mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "volume", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "mute", MPV_FORMAT_FLAG);

    const int initResult = mpv_initialize(m_mpv);
    appendMpvLog(QStringLiteral("createMpv: mpv_initialize() => %1 (%2)")
                     .arg(initResult)
                     .arg(mpvErrorString(initResult)));
    if (initResult < 0) {
        emit errorOccurred(QStringLiteral("mpv 初始化失败：%1").arg(mpvErrorString(initResult)));
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
        return;
    }
}

void MpvWidget::extractAudioWithFFmpeg(const QString &videoPath) {
    const QString outputPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/aiplayer_audio.wav";

    if (m_ffmpegProcess) {
        m_ffmpegProcess->kill();
        m_ffmpegProcess->waitForFinished(3000);
        m_ffmpegProcess->deleteLater();
        m_ffmpegProcess = nullptr;
    }

    if (QFile::exists(outputPath)) {
        QFile::remove(outputPath);
    }

    m_ffmpegProcess = new QProcess(this);
    m_ffmpegProcess->setProgram(QStringLiteral("ffmpeg"));
    m_ffmpegProcess->setArguments({
        QStringLiteral("-y"),
        QStringLiteral("-i"),
        videoPath,
        QStringLiteral("-ar"),
        QStringLiteral("16000"),
        QStringLiteral("-ac"),
        QStringLiteral("1"),
        QStringLiteral("-c:a"),
        QStringLiteral("pcm_s16le"),
        outputPath,
    });

    connect(m_ffmpegProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        if (!m_ffmpegProcess) {
            return;
        }
        const QByteArray data = m_ffmpegProcess->readAllStandardOutput();
        if (!data.isEmpty()) {
            std::fprintf(stderr, "%s", data.constData());
            std::fflush(stderr);
        }
    });

    connect(m_ffmpegProcess, &QProcess::readyReadStandardError, this, [this]() {
        if (!m_ffmpegProcess) {
            return;
        }
        const QByteArray data = m_ffmpegProcess->readAllStandardError();
        if (!data.isEmpty()) {
            std::fprintf(stderr, "%s", data.constData());
            std::fflush(stderr);
        }
    });

    connect(m_ffmpegProcess,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this,
            [this, outputPath](int exitCode, QProcess::ExitStatus exitStatus) {
                std::fprintf(stderr,
                             "[FFmpeg] Process finished with exitCode=%d, exitStatus=%d\n",
                             exitCode,
                             static_cast<int>(exitStatus));
                if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                    std::fprintf(stderr,
                                 "[FFmpeg] Audio extraction finished! %s is ready.\n",
                                 outputPath.toUtf8().constData());
                    
                    m_wavPath = outputPath;
                    {
                        std::lock_guard<std::mutex> lock(m_subtitleMutex);
                        m_subtitles.clear();
                        m_asrStatus = QStringLiteral("[ASR] 准备处理...");
                    }
                    
                    // Stop previous ASR thread if running
                    m_asrRunning.store(false);
                    if (m_asrThread.joinable()) m_asrThread.join();
                    
                    // Start new ASR thread
                    m_asrRunning.store(true);
                    m_asrThread = std::thread(&MpvWidget::runWhisper, this);
                }
                std::fflush(stderr);
                m_ffmpegProcess->deleteLater();
                m_ffmpegProcess = nullptr;
            });

    connect(m_ffmpegProcess,
            &QProcess::errorOccurred,
            this,
            [this](QProcess::ProcessError error) {
                emit errorOccurred(QStringLiteral("FFmpeg 音频提取失败：%1").arg(static_cast<int>(error)));
                m_ffmpegProcess->deleteLater();
                m_ffmpegProcess = nullptr;
            });

    std::fprintf(stderr,
                 "[FFmpeg] Starting extraction: %s -> %s\n",
                 videoPath.toUtf8().constData(),
                 outputPath.toUtf8().constData());
    std::fflush(stderr);
    m_ffmpegProcess->start();
}


void MpvWidget::initializeRenderContext() {
    if (!m_mpv || m_renderContext) {
        return;
    }

    QOpenGLContext *glContext = context();
    if (!glContext || !QOpenGLContext::currentContext()) {
        emit errorOccurred(QStringLiteral("OpenGL 上下文尚未就绪"));
        return;
    }

    appendMpvLog(QStringLiteral("initializeRenderContext: creating libmpv render context"));

    mpv_opengl_init_params glInitParams{};
    glInitParams.get_proc_address = &MpvWidget::getProcAddress;
    glInitParams.get_proc_address_ctx = glContext;

    int advancedControl = 1;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glInitParams},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &advancedControl},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    const int result = mpv_render_context_create(&m_renderContext, m_mpv, params);
    appendMpvLog(QStringLiteral("initializeRenderContext: mpv_render_context_create() => %1 (%2)")
                     .arg(result)
                     .arg(mpvErrorString(result)));
    if (result < 0) {
        emit errorOccurred(QStringLiteral("无法创建 mpv 渲染上下文：%1").arg(mpvErrorString(result)));
        m_renderContext = nullptr;
        return;
    }

    mpv_render_context_set_update_callback(m_renderContext, &MpvWidget::onUpdate, this);
    update();
}

void MpvWidget::handleMpvEvents() {
    if (!m_mpv) {
        return;
    }

    while (true) {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (!event || event->event_id == MPV_EVENT_NONE) {
            break;
        }

        if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
            auto *prop = static_cast<mpv_event_property *>(event->data);
            if (prop) {
                QByteArray name(prop->name);
                if (name == "pause" && prop->format == MPV_FORMAT_FLAG && prop->data) {
                    m_paused = *static_cast<int *>(prop->data) != 0;
                    emit playbackStateChanged(m_paused);
                } else if (name == "time-pos" && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
                    double pos = *static_cast<double *>(prop->data);
                    emit timePosChanged(pos);
                    
                    // Subtitle sync
                     qint64 posMs = static_cast<qint64>(pos * 1000);
                     QString currentText = "";
                     QString currentTranslated = "";
                     {
                         std::lock_guard<std::mutex> lock(m_subtitleMutex);
                         for (const auto &seg : m_subtitles) {
                             if (posMs >= seg.startMs && posMs <= seg.endMs) {
                                 currentText = seg.text;
                                 currentTranslated = seg.translatedText;
                                 break;
                             }
                         }
                         if (currentText.isEmpty() && !m_asrStatus.isEmpty()) {
                             currentText = m_asrStatus;
                         }
                     }
                     
                     emit asrTextUpdated(currentText, currentTranslated);
                } else if (name == "duration" && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
                     emit durationChanged(*static_cast<double *>(prop->data));
                 } else if (name == "volume" && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
                     emit volumeChanged(static_cast<int>(*static_cast<double *>(prop->data)));
                 } else if (name == "mute" && prop->format == MPV_FORMAT_FLAG && prop->data) {
                     emit muteStateChanged(*static_cast<int *>(prop->data) != 0);
                 }
            }
        } else if (event->event_id == MPV_EVENT_LOG_MESSAGE) {
            auto *logMessage = static_cast<mpv_event_log_message *>(event->data);
            if (logMessage) {
                appendMpvLog(QStringLiteral("[%1][%2] %3")
                                 .arg(QString::fromUtf8(logMessage->prefix ? logMessage->prefix : "mpv"))
                                 .arg(QString::fromUtf8(logMessage->level ? logMessage->level : "unknown"))
                                 .arg(QString::fromUtf8(logMessage->text ? logMessage->text : "").trimmed()));
            }
            continue;
        }
    }
}

void MpvWidget::requestUpdate() {
    QCoreApplication::postEvent(this, new QEvent(kMpvUpdateEvent));
}

void MpvWidget::renderFrame() {
    if (!m_renderContext || !context()) {
        return;
    }

    const qreal dpr = devicePixelRatioF();
    const int width = static_cast<int>(this->width() * dpr);
    const int height = static_cast<int>(this->height() * dpr);
    if (width <= 0 || height <= 0) {
        return;
    }

    int flipY = 1;
    const GLuint defaultFbo = defaultFramebufferObject();
    mpv_opengl_fbo fbo{
        .fbo = static_cast<int>(defaultFbo),
        .w = width,
        .h = height,
        .internal_format = 0,
    };

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flipY},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    QOpenGLFunctions *glFunctions = context()->functions();
    glFunctions->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    mpv_render_context_render(m_renderContext, params);
    glFunctions->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glFunctions->glFlush();
}

void MpvWidget::setPaused(bool paused) {
    if (!m_mpv) {
        return;
    }

    int value = paused ? 1 : 0;
    if (mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &value) < 0) {
        emit errorOccurred(QStringLiteral("设置播放状态失败"));
        return;
    }

    m_paused = paused;
    emit playbackStateChanged(m_paused);
}

void MpvWidget::seek(double pos) {
    if (!m_mpv) return;
    const char *args[] = {"seek", QByteArray::number(pos).constData(), "absolute", nullptr};
    mpv_command(m_mpv, args);
}

void MpvWidget::seekRelative(double pos) {
    if (!m_mpv) return;
    const char *args[] = {"seek", QByteArray::number(pos).constData(), "relative", nullptr};
    mpv_command(m_mpv, args);
}

void MpvWidget::setPlaybackSpeed(double speed) {
    if (!m_mpv) return;
    mpv_set_property(m_mpv, "speed", MPV_FORMAT_DOUBLE, &speed);
}

void MpvWidget::setVolume(int volume) {
    if (!m_mpv) return;
    double vol = volume;
    mpv_set_property(m_mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
}

void MpvWidget::setMute(bool mute) {
    if (!m_mpv) return;
    int val = mute ? 1 : 0;
    mpv_set_property(m_mpv, "mute", MPV_FORMAT_FLAG, &val);
}

void MpvWidget::updateAsrStatus(const QString &status) {
    std::lock_guard<std::mutex> lock(m_subtitleMutex);
    m_asrStatus = status;
}

void MpvWidget::runWhisper() {
    updateAsrStatus(QStringLiteral("[ASR] 准备加载模型..."));

    QSettings settings("AIPlayer", "Settings");
    int modelIndex = settings.value("model_index", 0).toInt();
    
    // Determine the expected model name based on settings
    QString expectedModelName = "ggml-tiny.bin";
    if (modelIndex == 1) expectedModelName = "ggml-base.bin";
    else if (modelIndex == 2) expectedModelName = "ggml-small.bin";
    else if (modelIndex == 3) expectedModelName = "ggml-medium.bin";
    else if (modelIndex == 4) expectedModelName = "ggml-large-v3.bin";

    // Try finding the model file
    QString modelPath;
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/" + expectedModelName,
        QDir::currentPath() + "/" + expectedModelName,
        QDir::homePath() + "/" + expectedModelName
    };

    for (const QString &path : searchPaths) {
        if (QFile::exists(path)) {
            modelPath = path;
            break;
        }
    }

    if (modelPath.isEmpty()) {
        updateAsrStatus(QStringLiteral("[ASR] 错误: 找不到 Whisper 模型 (%1)。请在设置中下载。").arg(expectedModelName)); 
        return;
    }

    updateAsrStatus(QStringLiteral("[ASR] 加载模型中..."));
    
    struct whisper_context_params cparams = whisper_context_default_params();
    struct whisper_context *ctx = whisper_init_from_file_with_params(modelPath.toUtf8().constData(), cparams);

    if (!ctx) {
        updateAsrStatus(QStringLiteral("[ASR] 错误: 模型加载失败"));
        return;
    }

    updateAsrStatus(QStringLiteral("[ASR] 识别中..."));

    QString sourceLang = settings.value("source_lang", "auto").toString();

    if (!readWavAndProcess(m_wavPath, ctx, sourceLang.toStdString())) {
        updateAsrStatus(QStringLiteral("[ASR] 错误: 处理音频失败"));
    } else {
        updateAsrStatus(QStringLiteral(""));
    }

    whisper_free(ctx);
    m_asrRunning.store(false);
}

bool MpvWidget::readWavAndProcess(const QString &wavPath, struct whisper_context *ctx, const std::string &language) {
    std::ifstream fin(wavPath.toStdString(), std::ios::binary);
    if (!fin.is_open()) return false;

    // Skip WAV header (44 bytes for standard canonical WAV)
    fin.seekg(44, std::ios::beg);

    std::vector<int16_t> pcm16;
    int16_t sample;
    while (fin.read(reinterpret_cast<char *>(&sample), sizeof(int16_t))) {
        pcm16.push_back(sample);
    }

    if (pcm16.empty()) return false;

    std::vector<float> pcmf32(pcm16.size());
    for (size_t i = 0; i < pcm16.size(); i++) {
        pcmf32[i] = static_cast<float>(pcm16[i]) / 32768.0f;
    }

    whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.print_progress   = false;
    wparams.print_special    = false;
    wparams.print_realtime   = false;
    wparams.print_timestamps = false;
    wparams.language         = language.c_str();
    wparams.n_threads        = std::min(4, (int)std::thread::hardware_concurrency());

    // Whisper limits processing to chunks, but whisper_full handles it internally
    // To provide real-time feedback, we can use a callback or process in chunks.
    // Let's use a callback for new segments
    
    // Pass 'this' as user data
    wparams.new_segment_callback_user_data = this;
    wparams.new_segment_callback = [](struct whisper_context *ctx, struct whisper_state *state, int n_new, void *user_data) {
        auto *self = static_cast<MpvWidget *>(user_data);
        if (!self->m_asrRunning.load()) return;
        
        const int n_segments = whisper_full_n_segments(ctx);
        int s0 = n_segments - n_new;
        if (s0 == 0) s0 = 0;
        
        for (int i = s0; i < n_segments; i++) {
            const char *text = whisper_full_get_segment_text(ctx, i);
            const int64_t t0 = whisper_full_get_segment_t0(ctx, i);
            const int64_t t1 = whisper_full_get_segment_t1(ctx, i);
            
            SubtitleSegment seg;
            seg.startMs = t0 * 10;
            seg.endMs = t1 * 10;
            seg.text = QString::fromUtf8(text);
            
            int newIndex = 0;
            {
                std::lock_guard<std::mutex> lock(self->m_subtitleMutex);
                newIndex = self->m_subtitles.size();
                self->m_subtitles.push_back(seg);
            }
            
            // Just for debugging/logging
            std::fprintf(stderr, "[ASR] Segment: [%d - %d] %s\n", (int)t0, (int)t1, text);
            
            emit self->segmentRecognized(newIndex, seg.text);
        }
    };

    if (whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()) != 0) {
        return false;
    }

    return true;
}

void MpvWidget::reTranslateAll() {
    std::vector<std::pair<int, QString>> segmentsToTranslate;

    {
        std::lock_guard<std::mutex> lock(m_subtitleMutex);
        for (size_t i = 0; i < m_subtitles.size(); ++i) {
            m_subtitles[i].translatedText.clear();
            segmentsToTranslate.push_back({static_cast<int>(i), m_subtitles[i].text});
        }
    }

    // Fire off translation requests for all existing segments based on the new settings
    for (const auto &pair : segmentsToTranslate) {
        emit segmentRecognized(pair.first, pair.second);
    }
}

void MpvWidget::translateSegment(int index, const QString &text) {
    QSettings settings("AIPlayer", "Settings");
    if (!settings.value("translation_enabled", false).toBool()) return;

    QString targetLang = settings.value("target_lang", "zh-CN").toString();
    QString sourceLang = settings.value("source_lang", "auto").toString();

    QUrl url("https://translate.googleapis.com/translate_a/single?client=gtx&sl=" + sourceLang + "&tl=" + targetLang + "&dt=t&q=" + QUrl::toPercentEncoding(text));
    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, index]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray arr = doc.array();
            QString result;
            if (!arr.isEmpty() && arr[0].isArray()) {
                QJsonArray lines = arr[0].toArray();
                for (int i = 0; i < lines.size(); ++i) {
                    if (lines[i].isArray()) {
                        result += lines[i].toArray()[0].toString();
                    }
                }
            }
            if (!result.isEmpty()) {
                std::lock_guard<std::mutex> lock(m_subtitleMutex);
                if (index >= 0 && index < static_cast<int>(m_subtitles.size())) {
                    m_subtitles[index].translatedText = result;
                }
            }
        }
    });
}

void MpvWidget::appendMpvLog(const QString &message) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    const QString logPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/mpv.log";
    std::ofstream stream(logPath.toStdString(), std::ios::app);
    if (!stream.is_open()) {
        return;
    }

    stream << QDateTime::currentDateTime().toString(Qt::ISODateWithMs).toStdString()
           << " "
           << message.toStdString()
           << std::endl;
}

QString MpvWidget::mpvErrorString(int errorCode) const {
    const char *error = mpv_error_string(errorCode);
    if (!error) {
        return QStringLiteral("unknown");
    }
    return QString::fromUtf8(error);
}
