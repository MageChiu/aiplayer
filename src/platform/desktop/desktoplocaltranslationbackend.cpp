#include "platform/desktop/desktoplocaltranslationbackend.h"

#include "libs/logging/include/logcenter.h"
#include "core/model/modelmanager.h"
#include "core/translation/localtranslationengine.h"

#include <QFile>
#include <QMetaObject>

namespace {
QString buildLocalTranslationPrompt(const QString &sourceLang, const QString &targetLang, const QString &text) {
    return QStringLiteral(
               "You are a professional subtitle translator.\n"
               "Translate the subtitle from %1 to %2.\n"
               "Do not explain. Do not repeat the source text. Do not add notes.\n"
               "Return only the translated subtitle wrapped in <translation> and </translation>.\n"
               "Keep punctuation and line breaks natural.\n"
               "<source>%3</source>\n"
               "<translation>")
        .arg(sourceLang, targetLang, text);
}
}

DesktopLocalTranslationBackend::DesktopLocalTranslationBackend(QObject *parent)
    : QObject(parent) {
}

DesktopLocalTranslationBackend::~DesktopLocalTranslationBackend() {
    cancelAll();
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

bool DesktopLocalTranslationBackend::supports(const TranslationSettings &settings) const {
    return settings.translationMode == QStringLiteral("local_gguf");
}

void DesktopLocalTranslationBackend::translate(const TranslationRequest &request,
                                               const TranslationSettings &settings,
                                               const TranslationSuccessCallback &onSuccess,
                                               const TranslationFailureCallback &onFailure) {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_pendingIndices.find(request.index) != m_pendingIndices.end()) {
            return;
        }
        m_pendingIndices.insert(request.index);
        m_queue.push(QueueItem{request, settings, onSuccess, onFailure});
    }
    processNext();
}

void DesktopLocalTranslationBackend::cancelAll() {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    std::queue<QueueItem> empty;
    std::swap(m_queue, empty);
    m_pendingIndices.clear();
}

void DesktopLocalTranslationBackend::processNext() {
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_running || m_queue.empty()) {
            return;
        }
        m_running = true;
    }

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    if (!m_localTranslationEngine) {
        m_localTranslationEngine = std::make_unique<LocalTranslationEngine>();
    }

    m_workerThread = std::thread([this]() {
        for (;;) {
            QueueItem item;
            {
                std::lock_guard<std::mutex> lock(m_queueMutex);
                if (m_queue.empty()) {
                    break;
                }
                item = m_queue.front();
                m_queue.pop();
            }

            const ModelManager modelManager;
            const ModelDescriptor descriptor = item.settings.localModelUrl.isEmpty()
                ? modelManager.translationModelAt(item.settings.localModelIndex)
                : modelManager.translationModelByUrl(item.settings.localModelUrl);
            const QString modelPath = item.settings.localModelPath.trimmed().isEmpty()
                ? modelManager.filePathFor(descriptor)
                : item.settings.localModelPath.trimmed();

            QString translatedText;
            QString errorMessage;
            if (!QFile::exists(modelPath)) {
                errorMessage = QStringLiteral("未找到本地翻译模型：%1").arg(modelPath);
            } else if (!m_localTranslationEngine->ensureModelLoaded(modelPath, &errorMessage)) {
            } else {
                translatedText = m_localTranslationEngine->translate(
                    buildLocalTranslationPrompt(item.request.sourceLanguage, item.request.targetLanguage, item.request.text),
                    256,
                    &errorMessage);
            }

            QMetaObject::invokeMethod(this, [this, item, translatedText, errorMessage]() {
                {
                    std::lock_guard<std::mutex> lock(m_queueMutex);
                    m_pendingIndices.erase(item.request.index);
                }

                if (!translatedText.isEmpty()) {
                    if (item.onSuccess) {
                        item.onSuccess(item.request.index, translatedText);
                    }
                } else if (item.onFailure) {
                    item.onFailure(item.request.index,
                                   errorMessage.isEmpty() ? QStringLiteral("本地离线翻译失败") : errorMessage);
                }
            }, Qt::QueuedConnection);
        }

        QMetaObject::invokeMethod(this, [this]() {
            m_running = false;
            processNext();
        }, Qt::QueuedConnection);
    });
}
