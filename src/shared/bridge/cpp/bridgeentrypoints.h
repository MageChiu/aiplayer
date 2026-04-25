#pragma once

#include "../../contracts/app/appcontracts.h"
#include "../../facade/player/playerfacade.h"
#include "../../facade/model/modelfacade.h"
#include "../../facade/translation/translationfacade.h"

struct SharedCppBridgeBindings {
    ISharedAppEventSink *appEventSink = nullptr;
    ISharedAppShellStateSource *appShellStateSource = nullptr;
    ISharedPlayerFacade *playerFacade = nullptr;
    ISharedTranslationFacade *translationFacade = nullptr;
    ISharedModelFacade *modelFacade = nullptr;
};
