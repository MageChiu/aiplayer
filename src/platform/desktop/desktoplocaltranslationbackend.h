#pragma once

#include "shared/contracts/translation/translationbackend.h"

#include <QObject>

#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_set>

class LocalTranslationEngine;

class DesktopLocalTranslationBackend : public QObject, public ITranslationBackend {
    Q_OBJECT

public:
    explicit DesktopLocalTranslationBackend(QObject *parent = nullptr);
    ~DesktopLocalTranslationBackend() override;

    bool supports(const TranslationSettings &settings) const override;
    void translate(const TranslationRequest &request,
                   const TranslationSettings &settings,
                   const TranslationSuccessCallback &onSuccess,
                   const TranslationFailureCallback &onFailure) override;
    void cancelAll() override;

private:
    struct QueueItem {
        TranslationRequest request;
        TranslationSettings settings;
        TranslationSuccessCallback onSuccess;
        TranslationFailureCallback onFailure;
    };

    void processNext();

    std::unique_ptr<LocalTranslationEngine> m_localTranslationEngine;
    std::queue<QueueItem> m_queue;
    std::unordered_set<int> m_pendingIndices;
    std::mutex m_queueMutex;
    std::thread m_workerThread;
    bool m_running = false;
};
