#include "mpvwidget.h"

#include <QCoreApplication>
#include <QEvent>
#include <QMetaObject>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <QTimer>

#include <chrono>

namespace {
constexpr QEvent::Type kMpvUpdateEvent = static_cast<QEvent::Type>(QEvent::User + 1);
}

MpvWidget::MpvWidget(QWidget *parent)
    : QOpenGLWidget(parent), m_eventTimer(new QTimer(this)) {
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    setMinimumSize(640, 360);

    connect(m_eventTimer, &QTimer::timeout, this, &MpvWidget::handleMpvEvents);
    m_eventTimer->setInterval(16);

    // Phase 2/3：先用 mock ASR 打通“识别->UI字幕”链路
    m_asrRunning.store(true);
    m_asrThread = std::thread([this]() {
        int idx = 0;
        while (m_asrRunning.load()) {
            const QString text = QStringLiteral("[ASR-MOCK] 识别中... %1").arg(idx++);
            QMetaObject::invokeMethod(this, [this, text]() { emit asrTextUpdated(text); }, Qt::QueuedConnection);
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        }
    });
}

MpvWidget::~MpvWidget() {
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

    initializeRenderContext();
    if (!m_renderContext) {
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
    initializePlayer();
}

void MpvWidget::paintGL() {
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
    self->requestUpdate();
}

void *MpvWidget::getProcAddress(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    if (auto *context = QOpenGLContext::currentContext()) {
        return reinterpret_cast<void *>(context->getProcAddress(name));
    }
    return nullptr;
}

void MpvWidget::createMpv() {
    if (m_mpv) {
        return;
    }

    m_mpv = mpv_create();
    if (!m_mpv) {
        emit errorOccurred(QStringLiteral("无法创建 mpv 实例"));
        return;
    }

    mpv_set_option_string(m_mpv, "terminal", "yes");
    mpv_set_option_string(m_mpv, "msg-level", "all=v");
    mpv_set_option_string(m_mpv, "vo", "libmpv");
    mpv_set_option_string(m_mpv, "hwdec", "auto-safe");
    mpv_set_wakeup_callback(m_mpv, &MpvWidget::onMpvWakeup, this);

    if (mpv_initialize(m_mpv) < 0) {
        emit errorOccurred(QStringLiteral("mpv 初始化失败"));
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }
}

void MpvWidget::initializeRenderContext() {
    if (!m_mpv || m_renderContext) {
        return;
    }

    if (!context()) {
        emit errorOccurred(QStringLiteral("OpenGL 上下文尚未就绪"));
        return;
    }

    makeCurrent();

    mpv_opengl_init_params glInitParams{};
    glInitParams.get_proc_address = &MpvWidget::getProcAddress;
    glInitParams.get_proc_address_ctx = this;

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glInitParams},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, reinterpret_cast<void *>(1)},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    if (mpv_render_context_create(&m_renderContext, m_mpv, params) < 0) {
        emit errorOccurred(QStringLiteral("无法创建 mpv 渲染上下文"));
        m_renderContext = nullptr;
        return;
    }

    mpv_render_context_set_update_callback(m_renderContext, &MpvWidget::onUpdate, this);
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
            if (prop && QByteArray(prop->name) == "pause" && prop->format == MPV_FORMAT_FLAG && prop->data) {
                m_paused = *static_cast<int *>(prop->data) != 0;
                emit playbackStateChanged(m_paused);
            }
        } else if (event->event_id == MPV_EVENT_LOG_MESSAGE) {
            continue;
        }
    }
}

void MpvWidget::requestUpdate() {
    QCoreApplication::postEvent(this, new QEvent(kMpvUpdateEvent));
}

void MpvWidget::renderFrame() {
    if (!m_renderContext) {
        return;
    }

    makeCurrent();

    const qreal dpr = devicePixelRatioF();
    const int width = static_cast<int>(this->width() * dpr);
    const int height = static_cast<int>(this->height() * dpr);
    int flipY = 1;

    mpv_opengl_fbo fbo{
        .fbo = 0,
        .w = width,
        .h = height,
        .internal_format = 0,
    };

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flipY},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    mpv_render_context_render(m_renderContext, params);
    context()->functions()->glFlush();
    doneCurrent();
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
