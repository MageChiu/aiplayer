#include "platform/desktop/onlinetranslationbackend.h"

#include "libs/logging/include/logcenter.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

namespace {
QString joinUrl(const QString &baseUrl, const QString &endpoint) {
    const QString normalizedBase = baseUrl.endsWith('/') ? baseUrl.left(baseUrl.size() - 1) : baseUrl;
    const QString normalizedEndpoint = endpoint.startsWith('/') ? endpoint : QStringLiteral("/") + endpoint;
    return normalizedBase + normalizedEndpoint;
}

QString parseGoogleCompatibleTranslation(const QByteArray &payload) {
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isArray()) {
        return {};
    }

    QString result;
    const QJsonArray root = doc.array();
    if (!root.isEmpty() && root.at(0).isArray()) {
        for (const auto &entry : root.at(0).toArray()) {
            if (!entry.isArray()) {
                continue;
            }
            const QJsonArray segment = entry.toArray();
            if (!segment.isEmpty()) {
                result += segment.at(0).toString();
            }
        }
    }
    return result.trimmed();
}

QString parseLibreTranslateTranslation(const QByteArray &payload) {
    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isObject()) {
        return {};
    }
    return doc.object().value(QStringLiteral("translatedText")).toString().trimmed();
}
}

OnlineTranslationBackend::OnlineTranslationBackend(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)) {
}

bool OnlineTranslationBackend::supports(const TranslationSettings &settings) const {
    return settings.translationMode == QStringLiteral("online");
}

void OnlineTranslationBackend::translate(const TranslationRequest &request,
                                         const TranslationSettings &settings,
                                         const TranslationSuccessCallback &onSuccess,
                                         const TranslationFailureCallback &onFailure) {
    QNetworkRequest networkRequest;
    QNetworkReply *reply = nullptr;

    if (settings.provider == QStringLiteral("libretranslate")) {
        networkRequest.setUrl(QUrl(joinUrl(settings.baseUrl, settings.endpoint)));
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        QJsonObject payload{
            {QStringLiteral("q"), request.text},
            {QStringLiteral("source"), request.sourceLanguage},
            {QStringLiteral("target"), request.targetLanguage},
            {QStringLiteral("format"), QStringLiteral("text")}
        };
        if (!settings.apiKey.trimmed().isEmpty()) {
            payload.insert(QStringLiteral("api_key"), settings.apiKey.trimmed());
        }
        reply = m_networkManager->post(networkRequest, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    } else {
        QString resolvedEndpoint = settings.endpoint;
        resolvedEndpoint.replace(QStringLiteral("{sl}"), request.sourceLanguage);
        resolvedEndpoint.replace(QStringLiteral("{tl}"), request.targetLanguage);
        resolvedEndpoint.replace(QStringLiteral("{q}"), QString::fromUtf8(QUrl::toPercentEncoding(request.text)));
        networkRequest.setUrl(QUrl(joinUrl(settings.baseUrl, resolvedEndpoint)));
        reply = m_networkManager->get(networkRequest);
    }

    LogCenter::instance().appendLog(QStringLiteral("online-translation"),
                                    QStringLiteral("请求翻译[%1]: %2").arg(request.index).arg(request.text.left(120)));

    connect(reply, &QNetworkReply::finished, this, [reply, request, onSuccess, onFailure]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (onFailure) {
                onFailure(request.index, QStringLiteral("在线翻译失败：%1").arg(reply->errorString()));
            }
            return;
        }

        const QByteArray payload = reply->readAll();
        const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower();
        const QString translated = contentType.contains(QStringLiteral("application/json")) &&
                                           payload.trimmed().startsWith('{')
                                       ? parseLibreTranslateTranslation(payload)
                                       : parseGoogleCompatibleTranslation(payload);
        if (translated.isEmpty()) {
            if (onFailure) {
                onFailure(request.index, QStringLiteral("在线翻译未返回可用文本"));
            }
            return;
        }

        if (onSuccess) {
            onSuccess(request.index, translated);
        }
    });
}

void OnlineTranslationBackend::cancelAll() {
    const auto replies = findChildren<QNetworkReply *>();
    for (QNetworkReply *reply : replies) {
        reply->abort();
    }
}
