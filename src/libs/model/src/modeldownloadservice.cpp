#include "libs/model/include/modeldownloadservice.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

ModelDownloadService::ModelDownloadService(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)) {
}

ModelDownloadService::~ModelDownloadService() {
    cancelDownload();
    resetActiveDownload();
}

bool ModelDownloadService::isDownloading() const {
    return m_currentReply != nullptr;
}

bool ModelDownloadService::startDownload(const DownloadTask &task) {
    if (isDownloading()) {
        return false;
    }

    const QFileInfo fileInfo(task.targetFilePath);
    QDir().mkpath(fileInfo.absolutePath());

    m_outputFile = new QFile(task.targetFilePath, this);
    if (!m_outputFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        delete m_outputFile;
        m_outputFile = nullptr;
        emit downloadFailed(task, QStringLiteral("无法创建文件：%1").arg(task.targetFilePath));
        return false;
    }

    m_activeTask = task;
    QNetworkRequest request(QUrl(task.sourceUrl));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_currentReply = m_networkManager->get(request);

    connect(m_currentReply, &QNetworkReply::readyRead, this, [this]() {
        if (m_outputFile) {
            m_outputFile->write(m_currentReply->readAll());
        }
    });
    connect(m_currentReply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal) {
        emit downloadProgress(m_activeTask, DownloadProgress{bytesReceived, bytesTotal});
    });
    connect(m_currentReply, &QNetworkReply::finished, this, [this]() {
        if (m_outputFile) {
            m_outputFile->write(m_currentReply->readAll());
            m_outputFile->close();
        }

        const DownloadTask finishedTask = m_activeTask;
        const QNetworkReply::NetworkError error = m_currentReply->error();
        const QString errorString = m_currentReply->errorString();
        resetActiveDownload();

        if (error == QNetworkReply::NoError) {
            emit downloadFinished(finishedTask);
            return;
        }

        QFile::remove(finishedTask.targetFilePath);
        if (error == QNetworkReply::OperationCanceledError) {
            emit downloadCancelled(finishedTask);
        } else {
            emit downloadFailed(finishedTask, errorString);
        }
    });

    return true;
}

void ModelDownloadService::cancelDownload() {
    if (m_currentReply) {
        m_currentReply->abort();
    }
}

void ModelDownloadService::resetActiveDownload() {
    if (m_currentReply) {
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    if (m_outputFile) {
        m_outputFile->close();
        delete m_outputFile;
        m_outputFile = nullptr;
    }
    m_activeTask = {};
}
