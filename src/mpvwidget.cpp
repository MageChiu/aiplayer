#include "libs/asr/include/asrservice.h"
#include "libs/logging/include/logcenter.h"
#include "core/path/apppaths.h"
#include "platform/desktop/desktopasrconfiguration.h"
#include "core/translation/translationservice.h"
#include "mpvwidget.h"
#include "shared/models/translation/translationsettings.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDataStream>
#include <QDir>
#include <QDirIterator>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <QTimer>
#include <QVector>

#include <chrono>
#include <clocale>
#include <cstdio>
#include <cstring>
#include <functional>
#include <mutex>

#include <QStandardPaths>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

#if AIPLAYER_ENABLE_TORRENT
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/settings_pack.hpp>
#include <libtorrent/torrent_flags.hpp>
#include <libtorrent/torrent_handle.hpp>

namespace lt = libtorrent;
#endif

namespace {
constexpr QEvent::Type kMpvUpdateEvent = static_cast<QEvent::Type>(QEvent::User + 1);

void syncTranslationStateConfig(TranslationState &state, const TranslationSettings &settings) {
    state.enabled = settings.translationEnabled;
    state.mode = settings.translationMode;
    state.provider = settings.provider;
    state.targetLanguage = settings.targetLanguage;

    if (!settings.translationEnabled) {
        state.status = TranslationStatus::Disabled;
        state.lastError.clear();
    } else if (state.status == TranslationStatus::Disabled) {
        state.status = TranslationStatus::Idle;
    }
}

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
        {QStringLiteral("data"), data},
        {QStringLiteral("ts"), QString::number(QDateTime::currentMSecsSinceEpoch())}
    };
    manager->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
}

QString ffmpegErrorString(int errorCode) {
    char buffer[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(errorCode, buffer, sizeof(buffer));
    return QString::fromUtf8(buffer);
}

bool isPlayableMediaFile(const QString &fileName) {
    const QString suffix = QFileInfo(fileName).suffix().toLower();
    static const QStringList kVideoSuffixes = {
        QStringLiteral("mp4"),
        QStringLiteral("mkv"),
        QStringLiteral("avi"),
        QStringLiteral("mov"),
        QStringLiteral("ts"),
        QStringLiteral("m2ts"),
        QStringLiteral("webm"),
        QStringLiteral("flv"),
        QStringLiteral("wmv")
    };
    return kVideoSuffixes.contains(suffix);
}

QString findLargestPlayableFile(const QString &rootDir) {
    QDirIterator it(rootDir, QDir::Files, QDirIterator::Subdirectories);
    QFileInfo best;
    while (it.hasNext()) {
        const QFileInfo info(it.next());
        if (!info.exists() || !info.isFile() || !isPlayableMediaFile(info.fileName())) {
            continue;
        }
        if (!best.exists() || info.size() > best.size()) {
            best = info;
        }
    }
    return best.exists() ? best.absoluteFilePath() : QString();
}

bool writeWaveHeader(QFile &file, quint32 sampleRate, quint16 channels, quint16 bitsPerSample, quint32 dataSize) {
    if (!file.isOpen()) {
        return false;
    }

    file.seek(0);
    const quint32 byteRate = sampleRate * channels * bitsPerSample / 8;
    const quint16 blockAlign = channels * bitsPerSample / 8;
    QByteArray header;
    QDataStream stream(&header, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData("RIFF", 4);
    stream << quint32(36 + dataSize);
    stream.writeRawData("WAVE", 4);
    stream.writeRawData("fmt ", 4);
    stream << quint32(16);
    stream << quint16(1);
    stream << quint16(channels);
    stream << quint32(sampleRate);
    stream << quint32(byteRate);
    stream << quint16(blockAlign);
    stream << quint16(bitsPerSample);
    stream.writeRawData("data", 4);
    stream << quint32(dataSize);
    return file.write(header) == header.size();
}

enum class AudioExtractResult {
    Success,
    Cancelled,
    Failed
};

AudioExtractResult extractAudioToWaveFile(const QString &inputPath,
                                         const QString &outputPath,
                                         std::atomic<bool> *cancelFlag,
                                         QString *errorMessage) {
    AVFormatContext *formatContext = nullptr;
    AVCodecContext *codecContext = nullptr;
    SwrContext *resampleContext = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
    const AVCodec *decoder = nullptr;
    int audioStreamIndex = -1;
    QFile outputFile(outputPath);
    quint32 dataSize = 0;
    AudioExtractResult result = AudioExtractResult::Failed;
#if LIBAVUTIL_VERSION_MAJOR >= 57
    AVChannelLayout outLayout{};
    bool outLayoutInitialized = false;
#endif

    const QByteArray inputUtf8 = inputPath.toUtf8();
    int fferr = avformat_open_input(&formatContext, inputUtf8.constData(), nullptr, nullptr);
    if (fferr < 0) {
        if (errorMessage) *errorMessage = ffmpegErrorString(fferr);
        goto cleanup;
    }

    fferr = avformat_find_stream_info(formatContext, nullptr);
    if (fferr < 0) {
        if (errorMessage) *errorMessage = ffmpegErrorString(fferr);
        goto cleanup;
    }

    audioStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &decoder, 0);
    if (audioStreamIndex < 0 || !decoder) {
        if (errorMessage) *errorMessage = QStringLiteral("未找到可用音频流");
        goto cleanup;
    }

    codecContext = avcodec_alloc_context3(decoder);
    if (!codecContext) {
        if (errorMessage) *errorMessage = QStringLiteral("无法创建 FFmpeg 解码上下文");
        goto cleanup;
    }

    fferr = avcodec_parameters_to_context(codecContext, formatContext->streams[audioStreamIndex]->codecpar);
    if (fferr < 0) {
        if (errorMessage) *errorMessage = ffmpegErrorString(fferr);
        goto cleanup;
    }

    fferr = avcodec_open2(codecContext, decoder, nullptr);
    if (fferr < 0) {
        if (errorMessage) *errorMessage = ffmpegErrorString(fferr);
        goto cleanup;
    }

    resampleContext = swr_alloc();
    if (!resampleContext) {
        if (errorMessage) *errorMessage = QStringLiteral("无法创建音频重采样器");
        goto cleanup;
    }

#if LIBAVUTIL_VERSION_MAJOR >= 57
    av_channel_layout_default(&outLayout, 1);
    outLayoutInitialized = true;
    av_opt_set_chlayout(resampleContext, "in_chlayout", &codecContext->ch_layout, 0);
    av_opt_set_chlayout(resampleContext, "out_chlayout", &outLayout, 0);
#else
    const int64_t inChannelLayout = codecContext->channel_layout != 0
        ? codecContext->channel_layout
        : av_get_default_channel_layout(codecContext->channels);
    av_opt_set_int(resampleContext, "in_channel_layout", inChannelLayout, 0);
    av_opt_set_int(resampleContext, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
#endif
    av_opt_set_int(resampleContext, "in_sample_rate", codecContext->sample_rate, 0);
    av_opt_set_int(resampleContext, "out_sample_rate", 16000, 0);
    av_opt_set_sample_fmt(resampleContext, "in_sample_fmt", codecContext->sample_fmt, 0);
    av_opt_set_sample_fmt(resampleContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    fferr = swr_init(resampleContext);
    if (fferr < 0) {
        if (errorMessage) *errorMessage = ffmpegErrorString(fferr);
        goto cleanup;
    }

    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) *errorMessage = QStringLiteral("无法创建输出 WAV 文件");
        goto cleanup;
    }
    outputFile.write(QByteArray(44, '\0'));

    packet = av_packet_alloc();
    frame = av_frame_alloc();
    if (!packet || !frame) {
        if (errorMessage) *errorMessage = QStringLiteral("无法创建 FFmpeg 数据缓冲");
        goto cleanup;
    }

    while ((fferr = av_read_frame(formatContext, packet)) >= 0) {
        if (cancelFlag && cancelFlag->load()) {
            result = AudioExtractResult::Cancelled;
            goto cleanup;
        }

        if (packet->stream_index != audioStreamIndex) {
            av_packet_unref(packet);
            continue;
        }

        fferr = avcodec_send_packet(codecContext, packet);
        av_packet_unref(packet);
        if (fferr < 0) {
            if (errorMessage) *errorMessage = ffmpegErrorString(fferr);
            goto cleanup;
        }

        while ((fferr = avcodec_receive_frame(codecContext, frame)) >= 0) {
            if (cancelFlag && cancelFlag->load()) {
                result = AudioExtractResult::Cancelled;
                goto cleanup;
            }

            const int outSamples = av_rescale_rnd(
                swr_get_delay(resampleContext, codecContext->sample_rate) + frame->nb_samples,
                16000,
                codecContext->sample_rate,
                AV_ROUND_UP);
            QVector<uint8_t> pcmBuffer(outSamples * 2);
            uint8_t *outData[] = { pcmBuffer.data() };

            const int convertedSamples = swr_convert(
                resampleContext,
                outData,
                outSamples,
                const_cast<const uint8_t **>(frame->extended_data),
                frame->nb_samples);
            if (convertedSamples < 0) {
                if (errorMessage) *errorMessage = ffmpegErrorString(convertedSamples);
                goto cleanup;
            }

            const int bytesToWrite = convertedSamples * 2;
            if (outputFile.write(reinterpret_cast<const char *>(pcmBuffer.constData()), bytesToWrite) != bytesToWrite) {
                if (errorMessage) *errorMessage = QStringLiteral("写入 WAV 数据失败");
                goto cleanup;
            }
            dataSize += bytesToWrite;
            av_frame_unref(frame);
        }

        if (fferr != AVERROR(EAGAIN) && fferr != AVERROR_EOF) {
            if (errorMessage) *errorMessage = ffmpegErrorString(fferr);
            goto cleanup;
        }
    }

    fferr = avcodec_send_packet(codecContext, nullptr);
    if (fferr >= 0) {
        while ((fferr = avcodec_receive_frame(codecContext, frame)) >= 0) {
            const int outSamples = av_rescale_rnd(
                swr_get_delay(resampleContext, codecContext->sample_rate) + frame->nb_samples,
                16000,
                codecContext->sample_rate,
                AV_ROUND_UP);
            QVector<uint8_t> pcmBuffer(outSamples * 2);
            uint8_t *outData[] = { pcmBuffer.data() };
            const int convertedSamples = swr_convert(
                resampleContext,
                outData,
                outSamples,
                const_cast<const uint8_t **>(frame->extended_data),
                frame->nb_samples);
            if (convertedSamples < 0) {
                if (errorMessage) *errorMessage = ffmpegErrorString(convertedSamples);
                goto cleanup;
            }
            const int bytesToWrite = convertedSamples * 2;
            if (outputFile.write(reinterpret_cast<const char *>(pcmBuffer.constData()), bytesToWrite) != bytesToWrite) {
                if (errorMessage) *errorMessage = QStringLiteral("写入 WAV 数据失败");
                goto cleanup;
            }
            dataSize += bytesToWrite;
            av_frame_unref(frame);
        }
    }

    if (!writeWaveHeader(outputFile, 16000, 1, 16, dataSize)) {
        if (errorMessage) *errorMessage = QStringLiteral("写入 WAV 文件头失败");
        goto cleanup;
    }
    result = AudioExtractResult::Success;

cleanup:
#if LIBAVUTIL_VERSION_MAJOR >= 57
    if (outLayoutInitialized) {
        av_channel_layout_uninit(&outLayout);
    }
#endif
    if (outputFile.isOpen()) {
        outputFile.close();
    }
    if (result != AudioExtractResult::Success) {
        QFile::remove(outputPath);
    }
    if (packet) av_packet_free(&packet);
    if (frame) av_frame_free(&frame);
    if (resampleContext) swr_free(&resampleContext);
    if (codecContext) avcodec_free_context(&codecContext);
    if (formatContext) avformat_close_input(&formatContext);
    return result;
}
}

#if AIPLAYER_ENABLE_TORRENT
class TorrentSessionController {
public:
    using PlayableCallback = std::function<void(const QString &, const QString &)>;
    using StatusCallback = std::function<void(const QString &)>;
    using ErrorCallback = std::function<void(const QString &)>;

    TorrentSessionController(QString magnetUri, QString downloadDirectory)
        : magnetUri_(std::move(magnetUri)), downloadDirectory_(std::move(downloadDirectory)) {}

    ~TorrentSessionController() { stop(); }

    void start(PlayableCallback onPlayable, StatusCallback onStatus, ErrorCallback onError) {
        onPlayable_ = std::move(onPlayable);
        onStatus_ = std::move(onStatus);
        onError_ = std::move(onError);
        stopRequested_.store(false);
        worker_ = std::thread(&TorrentSessionController::run, this);
    }

    void stop() {
        stopRequested_.store(true);
        if (session_) {
            session_->pause();
        }
        if (worker_.joinable()) {
            worker_.join();
        }
        session_.reset();
    }

private:
    void run() {
        QDir().mkpath(downloadDirectory_);

        lt::settings_pack settings;
        settings.set_int(lt::settings_pack::alert_mask,
                         lt::alert_category::error | lt::alert_category::status | lt::alert_category::storage);
        session_ = std::make_unique<lt::session>(settings);

        lt::error_code ec;
        lt::add_torrent_params params = lt::parse_magnet_uri(magnetUri_.toStdString(), ec);
        if (ec) {
            if (onError_) onError_(QStringLiteral("解析磁力链接失败：%1").arg(QString::fromStdString(ec.message())));
            return;
        }

        params.save_path = downloadDirectory_.toStdString();
        handle_ = session_->add_torrent(std::move(params), ec);
        if (ec) {
            if (onError_) onError_(QStringLiteral("启动磁力任务失败：%1").arg(QString::fromStdString(ec.message())));
            return;
        }

        handle_.set_flags(lt::torrent_flags::sequential_download);
        if (onStatus_) onStatus_(QStringLiteral("磁力任务已启动，正在获取元数据..."));

        while (!stopRequested_.load()) {
            std::vector<lt::alert *> alerts;
            session_->pop_alerts(&alerts);
            for (lt::alert *alert : alerts) {
                if (auto *metadata = lt::alert_cast<lt::metadata_received_alert>(alert)) {
                    Q_UNUSED(metadata);
                    if (onStatus_) onStatus_(QStringLiteral("已获取种子元数据，正在预缓冲视频..."));
                } else if (auto *torrentErr = lt::alert_cast<lt::torrent_error_alert>(alert)) {
                    if (onError_) onError_(QStringLiteral("磁力任务错误：%1").arg(QString::fromStdString(torrentErr->error.message())));
                }
            }

            if (!playableNotified_) {
                const QString mediaPath = findLargestPlayableFile(downloadDirectory_);
                const QFileInfo info(mediaPath);
                if (info.exists() && info.size() > 8 * 1024 * 1024) {
                    playableNotified_ = true;
                    if (onPlayable_) onPlayable_(info.absoluteFilePath(), info.fileName());
                    if (onStatus_) onStatus_(QStringLiteral("已进入边下边播模式"));
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    QString magnetUri_;
    QString downloadDirectory_;
    std::unique_ptr<lt::session> session_;
    lt::torrent_handle handle_;
    std::thread worker_;
    std::atomic<bool> stopRequested_{false};
    bool playableNotified_ = false;
    PlayableCallback onPlayable_;
    StatusCallback onStatus_;
    ErrorCallback onError_;
};
#endif

MpvWidget::MpvWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      m_eventTimer(new QTimer(this)),
      m_asrService(std::make_unique<AsrService>(resolveDesktopAsrModelPath)),
      m_translationService(std::make_unique<TranslationService>(this)) {
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    setMinimumSize(640, 360);

    connect(m_eventTimer, &QTimer::timeout, this, &MpvWidget::handleMpvEvents);
    m_eventTimer->setInterval(16);

    connect(this, &MpvWidget::segmentRecognized, this, &MpvWidget::translateSegment, Qt::QueuedConnection);
    connect(m_translationService.get(), &TranslationService::translationReady, this, &MpvWidget::applyTranslationResult);
    connect(m_translationService.get(), &TranslationService::translationFailed, this, &MpvWidget::handleTranslationFailure);
}

MpvWidget::~MpvWidget() {
    stopAudioExtraction();
    stopTorrentStreaming();

    {
        if (m_translationService) {
            m_translationService->clearPending();
        }
    }

    m_asrRunning.store(false);
    if (m_asrThread.joinable()) {
        m_asrThread.join();
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

void MpvWidget::stopAudioExtraction() {
    m_audioExtractCancel.store(true);
    if (m_audioExtractThread.joinable()) {
        m_audioExtractThread.join();
    }
}

void MpvWidget::stopTorrentStreaming() {
#if AIPLAYER_ENABLE_TORRENT
    if (m_torrentController) {
        m_torrentController->stop();
        m_torrentController.reset();
    }
#endif
}

void MpvWidget::loadFile(const QString &filePath) {
    // #region debug-point B:loadfile-entry
    reportOpenVideoCrashDebug(QStringLiteral("B"), QStringLiteral("mpvwidget.cpp:loadFile:entry"), QStringLiteral("entered loadFile"), {
        {QStringLiteral("filePath"), filePath},
        {QStringLiteral("exists"), QFileInfo::exists(filePath)},
        {QStringLiteral("initialized"), m_initialized},
        {QStringLiteral("mpvReady"), m_mpv != nullptr}
    });
    // #endregion
    if (!initializePlayer()) {
        emit errorOccurred(QStringLiteral("播放器初始化失败"));
        return;
    }

    {
        QSettings settings(QStringLiteral("AIPlayer"), QStringLiteral("Settings"));
        syncTranslationStateConfig(m_translationState, TranslationSettings::load(settings));
    }

    stopTorrentStreaming();

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
        // #region debug-point B:extract-audio-start
        reportOpenVideoCrashDebug(QStringLiteral("B"), QStringLiteral("mpvwidget.cpp:loadFile:extract"), QStringLiteral("starting audio extraction"), {
            {QStringLiteral("filePath"), filePath}
        });
        // #endregion
        extractAudioWithFFmpeg(filePath);
    }

    if (isMagnet) {
#if AIPLAYER_ENABLE_TORRENT
        const QString downloadRoot = QDir(AppPaths::torrentsDirectory())
                                         .absoluteFilePath(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd_hhmmss_zzz")));
        m_torrentController = std::make_unique<TorrentSessionController>(filePath, downloadRoot);
        m_torrentController->start(
            [this](const QString &mediaPath, const QString &displayName) {
                QMetaObject::invokeMethod(this, [this, mediaPath, displayName]() {
                    const QByteArray utf8 = mediaPath.toUtf8();
                    const char *cmd[] = {"loadfile", utf8.constData(), nullptr};
                    if (mpv_command(m_mpv, cmd) < 0) {
                        emit errorOccurred(QStringLiteral("无法加载磁力缓存文件：%1").arg(mediaPath));
                        return;
                    }
                    setPaused(false);
                    m_playbackState.loadedSource = mediaPath;
                    emit fileLoaded(QStringLiteral("磁力边播：%1").arg(displayName));
                }, Qt::QueuedConnection);
            },
            [this](const QString &status) {
                QMetaObject::invokeMethod(this, [this, status]() {
                    std::lock_guard<std::mutex> lock(m_subtitleMutex);
                    m_asrStatus = status;
                    emit asrTextUpdated(m_asrStatus, QString());
                }, Qt::QueuedConnection);
            },
            [this](const QString &error) {
                QMetaObject::invokeMethod(this, [this, error]() { emit errorOccurred(error); }, Qt::QueuedConnection);
            });
        return;
#else
        emit errorOccurred(QStringLiteral("当前构建未启用磁力播放支持，请安装 libtorrent-rasterbar 或以 AIPLAYER_ENABLE_TORRENT=OFF 重新配置。"));
        return;
#endif
    }

    const QByteArray utf8 = filePath.toUtf8();
    const char *cmd[] = {"loadfile", utf8.constData(), nullptr};
    // #region debug-point C:mpv-loadfile-before
    reportOpenVideoCrashDebug(QStringLiteral("C"), QStringLiteral("mpvwidget.cpp:loadFile:mpv-before"), QStringLiteral("calling mpv loadfile"), {
        {QStringLiteral("filePath"), filePath}
    });
    // #endregion
    const int loadResult = mpv_command(m_mpv, cmd);
    // #region debug-point C:mpv-loadfile-after
    reportOpenVideoCrashDebug(QStringLiteral("C"), QStringLiteral("mpvwidget.cpp:loadFile:mpv-after"), QStringLiteral("mpv loadfile returned"), {
        {QStringLiteral("result"), loadResult},
        {QStringLiteral("filePath"), filePath}
    });
    // #endregion
    if (loadResult < 0) {
        emit errorOccurred(QStringLiteral("无法加载文件：%1").arg(filePath));
        return;
    }

    setPaused(false);
    m_playbackState.loadedSource = filePath;
    emit fileLoaded(filePath);
}

void MpvWidget::play() {
    if (isEofReached()) {
        seek(0.0);
    }
    setPaused(false);
}

void MpvWidget::pause() {
    setPaused(true);
}

void MpvWidget::stop() {
    if (!m_mpv) {
        return;
    }
    seek(0.0);
    setPaused(true);
    emit timePosChanged(0.0);
}

void MpvWidget::replay() {
    if (!m_mpv) {
        return;
    }
    seek(0.0);
    setPaused(false);
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
    stopAudioExtraction();
    m_audioExtractCancel.store(false);

    if (QFile::exists(outputPath)) {
        QFile::remove(outputPath);
    }

    m_audioExtractThread = std::thread([this, videoPath, outputPath]() {
        QString err;
        const AudioExtractResult result = extractAudioToWaveFile(videoPath, outputPath, &m_audioExtractCancel, &err);
        QMetaObject::invokeMethod(this, [this, result, outputPath, err]() {
            if (result == AudioExtractResult::Cancelled) {
                return;
            }
            if (result != AudioExtractResult::Success) {
                emit errorOccurred(QStringLiteral("FFmpeg 音频抽取失败：%1").arg(err));
                return;
            }

            m_wavPath = outputPath;
            {
                std::lock_guard<std::mutex> lock(m_subtitleMutex);
                m_subtitles.clear();
                m_asrStatus = QStringLiteral("[ASR] 准备处理...");
            }

            m_asrRunning.store(false);
            if (m_asrThread.joinable()) m_asrThread.join();

            m_asrRunning.store(true);
            m_asrThread = std::thread(&MpvWidget::runWhisper, this);
        }, Qt::QueuedConnection);
    });
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
                    m_playbackState.paused = m_paused;
                    emit playbackStateChanged(m_paused);
                } else if (name == "time-pos" && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
                    double pos = *static_cast<double *>(prop->data);
                    m_playbackState.positionSeconds = pos;
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
                     m_playbackState.durationSeconds = *static_cast<double *>(prop->data);
                     emit durationChanged(*static_cast<double *>(prop->data));
                 } else if (name == "volume" && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
                     m_playbackState.volume = static_cast<int>(*static_cast<double *>(prop->data));
                     emit volumeChanged(static_cast<int>(*static_cast<double *>(prop->data)));
                 } else if (name == "mute" && prop->format == MPV_FORMAT_FLAG && prop->data) {
                     m_playbackState.muted = *static_cast<int *>(prop->data) != 0;
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
    mpv_opengl_fbo fbo{};
    fbo.fbo = static_cast<int>(defaultFbo);
    fbo.w = width;
    fbo.h = height;
    fbo.internal_format = 0;

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
    m_playbackState.paused = paused;
    emit playbackStateChanged(m_paused);
}

bool MpvWidget::isEofReached() const {
    if (!m_mpv) {
        return false;
    }

    int eofReached = 0;
    if (mpv_get_property(m_mpv, "eof-reached", MPV_FORMAT_FLAG, &eofReached) < 0) {
        return false;
    }
    return eofReached != 0;
}

void MpvWidget::seek(double pos) {
    if (!m_mpv) return;
    const QByteArray posArg = QByteArray::number(pos, 'f', 3);
    const char *args[] = {"seek", posArg.constData(), "absolute", nullptr};
    mpv_command(m_mpv, args);
}

void MpvWidget::seekRelative(double pos) {
    if (!m_mpv) return;
    const QByteArray posArg = QByteArray::number(pos, 'f', 3);
    const char *args[] = {"seek", posArg.constData(), "relative", nullptr};
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
    m_playbackState.volume = volume;
}

void MpvWidget::setMute(bool mute) {
    if (!m_mpv) return;
    int val = mute ? 1 : 0;
    mpv_set_property(m_mpv, "mute", MPV_FORMAT_FLAG, &val);
    m_playbackState.muted = mute;
}

void MpvWidget::updateAsrStatus(const QString &status) {
    std::lock_guard<std::mutex> lock(m_subtitleMutex);
    m_asrStatus = status;
    m_subtitleState.statusMessage = status;
}

void MpvWidget::runWhisper() {
    updateAsrStatus(QStringLiteral("[ASR] 准备加载模型..."));

    updateAsrStatus(QStringLiteral("[ASR] 识别中..."));
    const AsrSettings asrSettings = loadDesktopAsrSettings();

    const AsrResult result = m_asrService->transcribeWaveFile(m_wavPath, asrSettings, [this](const AsrSegment &segment) {
        SubtitleSegment subtitleSegment;
        subtitleSegment.startMs = segment.startMs;
        subtitleSegment.endMs = segment.endMs;
        subtitleSegment.text = segment.text;

        int newIndex = 0;
        {
            std::lock_guard<std::mutex> lock(m_subtitleMutex);
            newIndex = static_cast<int>(m_subtitles.size());
            m_subtitles.push_back(subtitleSegment);
        }
        emit segmentRecognized(newIndex, subtitleSegment.text);
    });

    if (!result.success) {
        updateAsrStatus(QStringLiteral("[ASR] 错误: %1").arg(result.errorMessage));
    } else {
        updateAsrStatus(QStringLiteral(""));
    }
    m_asrRunning.store(false);
}

void MpvWidget::reTranslateAll() {
    std::vector<std::pair<int, QString>> segmentsToTranslate;

    if (m_translationService) {
        m_translationService->clearPending();
    }

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
    const TranslationSettings translationSettings = TranslationSettings::load(settings);
    syncTranslationStateConfig(m_translationState, translationSettings);
    if (!translationSettings.translationEnabled) return;
    {
        std::lock_guard<std::mutex> subtitleLock(m_subtitleMutex);
        if (index >= 0 && index < static_cast<int>(m_subtitles.size()) && !m_subtitles[index].translatedText.isEmpty()) {
            return;
        }
    }
    if (m_translationService) {
        m_translationState.status = TranslationStatus::Requesting;
        m_translationState.lastError.clear();
        m_translationService->requestTranslation(index, text);
    }
}

void MpvWidget::emitCurrentSubtitleUpdate() {
    if (!m_mpv) {
        return;
    }

    double pos = 0.0;
    if (mpv_get_property(m_mpv, "time-pos", MPV_FORMAT_DOUBLE, &pos) < 0) {
        return;
    }

    const qint64 posMs = static_cast<qint64>(pos * 1000);
    QString currentText;
    QString currentTranslated;
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

    // #region debug-point C:emit-current-subtitle
    reportTranslationDisplayDebug(QStringLiteral("C"), QStringLiteral("mpvwidget.cpp:emitCurrentSubtitleUpdate"), QStringLiteral("emitting subtitle update"), {
        {QStringLiteral("currentText"), currentText},
        {QStringLiteral("currentTranslated"), currentTranslated},
        {QStringLiteral("posMs"), QString::number(posMs)}
    });
    // #endregion
    m_subtitleState.currentText = currentText;
    m_subtitleState.currentTranslatedText = currentTranslated;
    emit asrTextUpdated(currentText, currentTranslated);
}

void MpvWidget::applyTranslationResult(int index, const QString &translatedText) {
    QString originalText;
    {
        std::lock_guard<std::mutex> lock(m_subtitleMutex);
        if (index < 0 || index >= static_cast<int>(m_subtitles.size())) {
            return;
        }
        originalText = m_subtitles[index].text;
        m_subtitles[index].translatedText = translatedText;
    }
    // #region debug-point B:apply-translation-result
    reportTranslationDisplayDebug(QStringLiteral("B"), QStringLiteral("mpvwidget.cpp:applyTranslationResult"), QStringLiteral("applied translation result"), {
        {QStringLiteral("index"), index},
        {QStringLiteral("originalText"), originalText},
        {QStringLiteral("translatedText"), translatedText}
    });
    // #endregion
    m_translationState.status = TranslationStatus::Success;
    m_translationState.lastError.clear();
    m_subtitleState.currentTranslatedText = translatedText;
    emitCurrentSubtitleUpdate();
}

void MpvWidget::handleTranslationFailure(const QString &message) {
    m_translationState.status = TranslationStatus::Failed;
    m_translationState.lastError = message;
    LogCenter::instance().appendLog(QStringLiteral("translation"), message);
    LogCenter::instance().setStatus(QStringLiteral("翻译状态"), message);
    emit errorOccurred(message);
}

void MpvWidget::appendMpvLog(const QString &message) {
    LogCenter::instance().appendLog(QStringLiteral("mpv"), message);
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
