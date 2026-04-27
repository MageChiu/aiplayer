#pragma once

#include <QObject>

class QFile;
class QNetworkAccessManager;
class QNetworkReply;

struct DownloadTask {
    QString sourceUrl;
    QString targetFilePath;
    QString displayName;
};

struct DownloadProgress {
    qint64 bytesReceived = 0;
    qint64 bytesTotal = 0;

    int percentage() const {
        if (bytesTotal <= 0) {
            return -1;
        }
        return static_cast<int>((bytesReceived * 100) / bytesTotal);
    }
};

class ModelDownloadService : public QObject {
    Q_OBJECT

public:
    explicit ModelDownloadService(QObject *parent = nullptr);
    ~ModelDownloadService() override;

    bool isDownloading() const;
    bool startDownload(const DownloadTask &task);
    void cancelDownload();

signals:
    void downloadProgress(const DownloadTask &task, const DownloadProgress &progress);
    void downloadFinished(const DownloadTask &task);
    void downloadFailed(const DownloadTask &task, const QString &message);
    void downloadCancelled(const DownloadTask &task);

private:
    void resetActiveDownload();

    QNetworkAccessManager *m_networkManager = nullptr;
    QNetworkReply *m_currentReply = nullptr;
    QFile *m_outputFile = nullptr;
    DownloadTask m_activeTask;
};
