#include "libs/asr/include/asrservice.h"

#include "whisper.h"

#include <algorithm>
#include <fstream>
#include <thread>
#include <vector>

namespace {
struct CallbackContext {
    AsrResult *result = nullptr;
    AsrService::SegmentCallback callback;
};
}

AsrService::AsrService(ModelPathResolver modelPathResolver)
    : m_modelPathResolver(std::move(modelPathResolver)) {
}

QString AsrService::resolveModelPath(const AsrSettings &settings) const {
    return m_modelPathResolver ? m_modelPathResolver(settings.modelIndex) : QString();
}

void AsrService::setModelPathResolver(ModelPathResolver modelPathResolver) {
    m_modelPathResolver = std::move(modelPathResolver);
}

AsrResult AsrService::transcribeWaveFile(const QString &wavPath,
                                         const AsrSettings &settings,
                                         const SegmentCallback &callback) const {
    AsrResult result;
    const QString modelPath = resolveModelPath(settings);
    if (modelPath.isEmpty()) {
        result.errorMessage = QStringLiteral("找不到 Whisper 模型，请在设置中下载对应模型。");
        return result;
    }

    whisper_context_params contextParams = whisper_context_default_params();
    whisper_context *context = whisper_init_from_file_with_params(modelPath.toUtf8().constData(), contextParams);
    if (!context) {
        result.errorMessage = QStringLiteral("Whisper 模型加载失败：%1").arg(modelPath);
        return result;
    }

    result.success = processWaveFile(wavPath, context, settings.sourceLanguage, &result, callback);
    if (!result.success && result.errorMessage.isEmpty()) {
        result.errorMessage = QStringLiteral("Whisper 处理音频失败");
    }

    whisper_free(context);
    return result;
}

bool AsrService::processWaveFile(const QString &wavPath,
                                 whisper_context *context,
                                 const QString &language,
                                 AsrResult *result,
                                 const SegmentCallback &callback) const {
    std::ifstream stream(wavPath.toStdString(), std::ios::binary);
    if (!stream.is_open()) {
        result->errorMessage = QStringLiteral("无法打开 WAV 文件：%1").arg(wavPath);
        return false;
    }

    stream.seekg(44, std::ios::beg);

    std::vector<int16_t> pcm16;
    int16_t sample = 0;
    while (stream.read(reinterpret_cast<char *>(&sample), sizeof(int16_t))) {
        pcm16.push_back(sample);
    }
    if (pcm16.empty()) {
        result->errorMessage = QStringLiteral("WAV 文件中没有可识别音频数据");
        return false;
    }

    std::vector<float> pcmf32(pcm16.size());
    for (size_t i = 0; i < pcm16.size(); ++i) {
        pcmf32[i] = static_cast<float>(pcm16[i]) / 32768.0f;
    }

    CallbackContext callbackContext{result, callback};
    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_progress = false;
    params.print_special = false;
    params.print_realtime = false;
    params.print_timestamps = false;
    const QByteArray languageUtf8 = language.toUtf8();
    params.language = languageUtf8.constData();
    params.n_threads = std::max(1, std::min(4, static_cast<int>(std::thread::hardware_concurrency())));
    params.new_segment_callback_user_data = &callbackContext;
    params.new_segment_callback = [](whisper_context *ctx, whisper_state *, int nNew, void *userData) {
        auto *callbackData = static_cast<CallbackContext *>(userData);
        const int totalSegments = whisper_full_n_segments(ctx);
        for (int i = std::max(0, totalSegments - nNew); i < totalSegments; ++i) {
            AsrSegment segment;
            segment.startMs = whisper_full_get_segment_t0(ctx, i) * 10;
            segment.endMs = whisper_full_get_segment_t1(ctx, i) * 10;
            segment.text = QString::fromUtf8(whisper_full_get_segment_text(ctx, i)).trimmed();
            callbackData->result->segments.push_back(segment);
            if (callbackData->callback) {
                callbackData->callback(segment);
            }
        }
    };

    if (whisper_full(context, params, pcmf32.data(), static_cast<int>(pcmf32.size())) != 0) {
        result->errorMessage = QStringLiteral("Whisper 推理失败");
        return false;
    }

    return true;
}
