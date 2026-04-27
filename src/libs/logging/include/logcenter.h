#pragma once

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QMutex>

class LogCenter : public QObject {
    Q_OBJECT

public:
    static LogCenter &instance();

    void appendLog(const QString &category, const QString &message);
    void setStatus(const QString &key, const QString &value);
    QStringList logs() const;
    QString statusText() const;
    void clear();

signals:
    void logAdded(const QString &line);
    void statusChanged(const QString &text);
    void logsCleared();

private:
    explicit LogCenter(QObject *parent = nullptr);
    QString buildStatusTextLocked() const;

    mutable QMutex m_mutex;
    QStringList m_logs;
    QStringList m_statusOrder;
    QHash<QString, QString> m_status;
};
