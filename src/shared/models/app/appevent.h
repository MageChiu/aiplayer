#pragma once

#include <QDateTime>
#include <QString>

struct AppEvent {
    QString category;
    QString message;
    QDateTime timestamp = QDateTime::currentDateTimeUtc();
};

struct LogEvent : public AppEvent {
    QString level;
};

struct ErrorEvent : public AppEvent {
    QString source;
    QString details;
};
