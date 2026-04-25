#pragma once

#include <QtCore/QString>
#include <QtGlobal>

struct SubtitleSegment {
    qint64 startMs = 0;
    qint64 endMs = 0;
    QString text;
    QString translatedText;
};
