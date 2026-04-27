#pragma once

#include "../../contracts/player/playercontracts.h"
#include "../../contracts/subtitle/subtitlecontracts.h"
#include "../../models/state/statevocabulary.h"

class ISharedPlayerFacade : public ISharedPlayerStateSource, public ISharedSubtitleTrackSource {
public:
    virtual ~ISharedPlayerFacade() = default;

    virtual bool hasLoadedSource() const = 0;
    virtual NavigationStateSnapshot recommendedRoute() const = 0;
};
