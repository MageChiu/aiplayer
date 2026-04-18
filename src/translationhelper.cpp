#include <QByteArray>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>

#include <llama.h>

#include <array>
#include <iostream>
#include <iterator>
#include <mutex>
#include <vector>

namespace {
QString sanitizeLocalTranslationOutput(const QString &text) {
    QString result = text.trimmed();

    result.remove(QRegularExpression(QStringLiteral("<think>[\\s\\S]*?</think>"), QRegularExpression::CaseInsensitiveOption));
    result.remove(QRegularExpression(QStringLiteral("```[\\s\\S]*?```")));

    const QRegularExpression translationTagRe(
        QStringLiteral("<translation>([\\s\\S]*?)</translation>"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch tagMatch = translationTagRe.match(result);
    if (tagMatch.hasMatch()) {
        result = tagMatch.captured(1).trimmed();
    }

    if (result.startsWith(QStringLiteral("assistant"), Qt::CaseInsensitive)) {
        const int colon = result.indexOf(':');
        if (colon >= 0) {
            result = result.mid(colon + 1).trimmed();
        }
    }

    const QStringList lines = result.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        static const QStringList prefixes = {
            QStringLiteral("translation:"),
            QStringLiteral("translated:"),
            QStringLiteral("译文："),
            QStringLiteral("译文:"),
            QStringLiteral("翻译："),
            QStringLiteral("翻译:")
        };
        for (const QString &prefix : prefixes) {
            if (trimmed.startsWith(prefix, Qt::CaseInsensitive)) {
                result = trimmed.mid(prefix.size()).trimmed();
                break;
            }
        }
    }

    result.remove(QRegularExpression(QStringLiteral("^['\"“”‘’]+|['\"“”‘’]+$")));
    result.remove(QRegularExpression(QStringLiteral("^<translation>|</translation>$"), QRegularExpression::CaseInsensitiveOption));
    return result;
}


bool loadModel(const QString &modelPath, llama_model **model, QString *errorMessage) {
    static std::once_flag backendInitFlag;
    std::call_once(backendInitFlag, []() {
        llama_backend_init();
    });

    llama_model_params modelParams = llama_model_default_params();
    *model = llama_model_load_from_file(modelPath.toUtf8().constData(), modelParams);
    if (!*model) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("加载本地翻译模型失败：%1").arg(modelPath);
        }
        return false;
    }
    return true;
}

QString translateWithModel(llama_model *model, const QString &prompt, int maxTokens, QString *errorMessage) {
    llama_context_params contextParams = llama_context_default_params();
    llama_context *ctx = llama_init_from_model(model, contextParams);
    if (!ctx) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("初始化本地翻译上下文失败");
        }
        return QString();
    }

    llama_sampler *sampler = llama_sampler_init_greedy();
    if (!sampler) {
        llama_free(ctx);
        if (errorMessage) {
            *errorMessage = QStringLiteral("初始化本地翻译采样器失败");
        }
        return QString();
    }

    const llama_vocab *vocab = llama_model_get_vocab(model);
    const QByteArray promptUtf8 = prompt.toUtf8();
    std::vector<llama_token> promptTokens(static_cast<size_t>(promptUtf8.size()) + 512);
    const int32_t promptTokenCount = llama_tokenize(
        vocab,
        promptUtf8.constData(),
        static_cast<int32_t>(promptUtf8.size()),
        promptTokens.data(),
        static_cast<int32_t>(promptTokens.size()),
        true,
        true
    );
    if (promptTokenCount < 0) {
        llama_sampler_free(sampler);
        llama_free(ctx);
        if (errorMessage) {
            *errorMessage = QStringLiteral("本地翻译提示词分词失败");
        }
        return QString();
    }
    promptTokens.resize(static_cast<size_t>(promptTokenCount));

    llama_batch promptBatch = llama_batch_get_one(promptTokens.data(), static_cast<int32_t>(promptTokens.size()));
    if (llama_decode(ctx, promptBatch) < 0) {
        llama_sampler_free(sampler);
        llama_free(ctx);
        if (errorMessage) {
            *errorMessage = QStringLiteral("本地翻译提示词推理失败");
        }
        return QString();
    }

    QByteArray outputBytes;
    std::array<char, 1024> pieceBuffer{};
    for (int i = 0; i < maxTokens; ++i) {
        const llama_token token = llama_sampler_sample(sampler, ctx, -1);
        if (llama_vocab_is_eog(vocab, token)) {
            break;
        }

        const int32_t pieceLength = llama_token_to_piece(
            vocab,
            token,
            pieceBuffer.data(),
            static_cast<int32_t>(pieceBuffer.size()),
            0,
            true
        );
        if (pieceLength > 0) {
            outputBytes.append(pieceBuffer.data(), pieceLength);
        }

        llama_sampler_accept(sampler, token);
        llama_batch tokenBatch = llama_batch_get_one(const_cast<llama_token *>(&token), 1);
        if (llama_decode(ctx, tokenBatch) < 0) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("本地翻译生成过程中推理失败");
            }
            outputBytes.clear();
            break;
        }
    }

    llama_sampler_free(sampler);
    llama_free(ctx);
    return sanitizeLocalTranslationOutput(QString::fromUtf8(outputBytes));
}
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("aiplayer-translate-helper"));

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption modelOption(QStringList() << "m" << "model", QStringLiteral("GGUF model path"), QStringLiteral("path"));
    QCommandLineOption maxTokensOption(QStringList() << "n" << "max-tokens", QStringLiteral("Maximum output tokens"), QStringLiteral("tokens"), QStringLiteral("256"));
    parser.addOption(modelOption);
    parser.addOption(maxTokensOption);
    parser.process(app);

    QTextStream err(stderr);
    QTextStream out(stdout);

    const QString modelPath = parser.value(modelOption).trimmed();
    const int maxTokens = parser.value(maxTokensOption).toInt();
    if (modelPath.isEmpty() || !QFileInfo::exists(modelPath)) {
        err << QStringLiteral("invalid model path: %1\n").arg(modelPath);
        return 2;
    }

    const QString prompt = QString::fromUtf8(QByteArray::fromStdString(std::string(
        std::istreambuf_iterator<char>(std::cin),
        std::istreambuf_iterator<char>())));
    if (prompt.trimmed().isEmpty()) {
        err << QStringLiteral("empty prompt\n");
        return 3;
    }

    llama_model *model = nullptr;
    QString errorMessage;
    if (!loadModel(modelPath, &model, &errorMessage)) {
        err << errorMessage << '\n';
        return 4;
    }

    const QString translated = translateWithModel(model, prompt, maxTokens > 0 ? maxTokens : 256, &errorMessage);
    llama_model_free(model);

    if (translated.isEmpty()) {
        err << (errorMessage.isEmpty() ? QStringLiteral("empty translation result") : errorMessage) << '\n';
        return 5;
    }

    out << translated;
    out.flush();
    return 0;
}
