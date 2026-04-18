#include "logcenter.h"

#include <QDateTime>
#include <QHash>
#include <QMutexLocker>

LogCenter::LogCenter(QObject *parent)
    : QObject(parent) {
}

LogCenter &LogCenter::instance() {
    static LogCenter instance;
    return instance;
}

void LogCenter::appendLog(const QString &category, const QString &message) {
    const QString line = QStringLiteral("[%1][%2] %3")
                             .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")))
                             .arg(category)
                             .arg(message);
    {
        QMutexLocker locker(&m_mutex);
        m_logs.append(line);
        if (m_logs.size() > 1000) {
            m_logs.remove(0, m_logs.size() - 1000);
        }
    }
    emit logAdded(line);
}

void LogCenter::setStatus(const QString &key, const QString &value) {
    QString text;
    {
        QMutexLocker locker(&m_mutex);
        if (!m_status.contains(key)) {
            m_statusOrder.append(key);
        }
        m_status.insert(key, value);
        text = buildStatusTextLocked();
    }
    emit statusChanged(text);
}

QStringList LogCenter::logs() const {
    QMutexLocker locker(&m_mutex);
    return m_logs;
}

QString LogCenter::statusText() const {
    QMutexLocker locker(&m_mutex);
    return buildStatusTextLocked();
}

void LogCenter::clear() {
    {
        QMutexLocker locker(&m_mutex);
        m_logs.clear();
        m_status.clear();
        m_statusOrder.clear();
    }
    emit logsCleared();
    emit statusChanged(QString());
}

QString LogCenter::buildStatusTextLocked() const {
    QStringList parts;
    for (const QString &key : m_statusOrder) {
        if (m_status.contains(key)) {
            parts << QStringLiteral("%1: %2").arg(key, m_status.value(key));
        }
    }
    return parts.join(QStringLiteral("\n"));
}
