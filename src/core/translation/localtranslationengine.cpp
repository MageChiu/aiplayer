#include "localtranslationengine.h"

#include "libs/logging/include/logcenter.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>

namespace {
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
        {QStringLiteral("data"), data}
    };
    manager->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
}

QString sanitizeLocalTranslationOutput(const QString &text) {
    QString result = text.trimmed();

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
    static const QStringList prefixes = {
        QStringLiteral("translation:"),
        QStringLiteral("translated:"),
        QStringLiteral("译文："),
        QStringLiteral("译文:"),
        QStringLiteral("翻译："),
        QStringLiteral("翻译:")
    };
    for (const QString &prefix : prefixes) {
        if (result.startsWith(prefix, Qt::CaseInsensitive)) {
            result = result.mid(prefix.size()).trimmed();
            break;
        }
    }
    result.remove(QRegularExpression(QStringLiteral("^['\"“”‘’]+|['\"“”‘’]+$")));
    return result;
}
}

LocalTranslationEngine::LocalTranslationEngine() = default;

LocalTranslationEngine::~LocalTranslationEngine() = default;

bool LocalTranslationEngine::ensureModelLoaded(const QString &modelPath, QString *errorMessage) {
    if (!QFileInfo::exists(modelPath)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("加载本地翻译模型失败：%1").arg(modelPath);
        }
        LogCenter::instance().appendLog(QStringLiteral("local-translation"), QStringLiteral("模型不存在：%1").arg(modelPath));
        return false;
    }
    m_modelPath = modelPath;
    LogCenter::instance().setStatus(QStringLiteral("离线模型"), modelPath);
    return true;
}

QString LocalTranslationEngine::resolveHelperPath() const {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir(appDir).absoluteFilePath(QStringLiteral("aiplayer-translate-helper")),
        QDir(appDir).absoluteFilePath(QStringLiteral("aiplayer-translate-helper.exe")),
        QDir(appDir).absoluteFilePath(QStringLiteral("../MacOS/aiplayer-translate-helper")),
        QDir(appDir).absoluteFilePath(QStringLiteral("../MacOS/aiplayer-translate-helper.exe"))
    };

    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate) && QFileInfo(candidate).isFile()) {
            return QDir::cleanPath(candidate);
        }
    }
    return QString();
}

QString LocalTranslationEngine::translate(const QString &prompt, int maxTokens, QString *errorMessage) {
    if (m_modelPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("本地翻译模型尚未加载");
        }
        return QString();
    }

    const QString helperPath = resolveHelperPath();
    if (helperPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("未找到本地翻译 helper：aiplayer-translate-helper");
        }
        LogCenter::instance().appendLog(QStringLiteral("local-translation"), QStringLiteral("未找到 helper 可执行文件"));
        return QString();
    }

    QProcess process;
    process.setProgram(helperPath);
    process.setArguments({
        QStringLiteral("--model"), m_modelPath,
        QStringLiteral("--max-tokens"), QString::number(maxTokens > 0 ? maxTokens : 256)
    });
    LogCenter::instance().appendLog(QStringLiteral("local-translation"),
                                    QStringLiteral("启动 helper：%1 --model %2 --max-tokens %3")
                                        .arg(helperPath, m_modelPath)
                                        .arg(maxTokens > 0 ? maxTokens : 256));
    process.start();
    if (!process.waitForStarted(3000)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法启动本地翻译 helper：%1").arg(helperPath);
        }
        return QString();
    }

    process.write(prompt.toUtf8());
    process.closeWriteChannel();
    if (!process.waitForFinished(120000)) {
        process.kill();
        process.waitForFinished(3000);
        if (errorMessage) {
            *errorMessage = QStringLiteral("本地翻译 helper 执行超时");
        }
        return QString();
    }

    const QString rawStdout = QString::fromUtf8(process.readAllStandardOutput());
    const QString output = sanitizeLocalTranslationOutput(rawStdout);
    const QString stderrOutput = QString::fromUtf8(process.readAllStandardError()).trimmed();
    LogCenter::instance().appendLog(QStringLiteral("local-translation"),
                                    QStringLiteral("helper 返回：exit=%1 stdout=%2 stderr=%3")
                                        .arg(process.exitCode())
                                        .arg(output.isEmpty() ? QStringLiteral("<empty>") : output.left(300))
                                        .arg(stderrOutput.isEmpty() ? QStringLiteral("<empty>") : stderrOutput.left(300)));
    // #region debug-point A:helper-result
    reportTranslationDisplayDebug(QStringLiteral("A"), QStringLiteral("localtranslationengine.cpp:translate"), QStringLiteral("helper finished"), {
        {QStringLiteral("exitCode"), process.exitCode()},
        {QStringLiteral("normalExit"), process.exitStatus() == QProcess::NormalExit},
        {QStringLiteral("rawStdout"), rawStdout},
        {QStringLiteral("sanitizedOutput"), output},
        {QStringLiteral("stderr"), stderrOutput}
    });
    // #endregion
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0 || output.isEmpty()) {
        if (errorMessage) {
            *errorMessage = stderrOutput.isEmpty()
                ? QStringLiteral("本地翻译 helper 未返回可用文本")
                : stderrOutput;
        }
        return QString();
    }

    return output;
}
