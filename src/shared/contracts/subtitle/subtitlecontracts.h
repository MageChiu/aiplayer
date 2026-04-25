#pragma once

#include "../../models/subtitle/subtitlesegment.h"
#include "../../models/state/statevocabulary.h"

#include <QVector>

struct SubtitleTrackSnapshot {
    AsyncLoadState loadState = AsyncLoadState::Idle;
    QVector<SubtitleSegment> segments;
    QString lastError;
};

class ISharedSubtitleTrackSource {
public:
    virtual ~ISharedSubtitleTrackSource() = default;

    virtual SubtitleTrackSnapshot subtitleTrack() const = 0;
};
