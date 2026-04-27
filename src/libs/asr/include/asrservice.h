#pragma once

#include <QVector>
#include <QString>
#include <QtGlobal>

#include <functional>

struct whisper_context;

struct AsrSegment {
    qint64 startMs = 0;
    qint64 endMs = 0;
    QString text;
};

struct AsrResult {
    bool success = false;
    QString errorMessage;
    QVector<AsrSegment> segments;
};

struct AsrSettings {
    int modelIndex = 0;
    QString sourceLanguage = QStringLiteral("auto");
};

class AsrService {
public:
    using SegmentCallback = std::function<void(const AsrSegment &)>;
    using ModelPathResolver = std::function<QString(int modelIndex)>;

    explicit AsrService(ModelPathResolver modelPathResolver = {});

    void setModelPathResolver(ModelPathResolver modelPathResolver);

    AsrResult transcribeWaveFile(const QString &wavPath, const AsrSettings &settings, const SegmentCallback &callback = {}) const;

private:
    QString resolveModelPath(const AsrSettings &settings) const;
    bool processWaveFile(const QString &wavPath,
                         whisper_context *context,
                         const QString &language,
                         AsrResult *result,
                         const SegmentCallback &callback) const;

    ModelPathResolver m_modelPathResolver;
};
