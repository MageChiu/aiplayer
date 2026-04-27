#pragma once

#include "../../models/state/appstate.h"

class ISharedPlayerStateSource {
public:
    virtual ~ISharedPlayerStateSource() = default;

    virtual PlaybackState playbackState() const = 0;
    virtual SubtitleState subtitleState() const = 0;
    virtual TranslationState translationState() const = 0;
};
