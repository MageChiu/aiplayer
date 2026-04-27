#pragma once

#include "../../models/app/appevent.h"
#include "../../models/state/appstate.h"
#include "../../models/state/statevocabulary.h"

class ISharedAppEventSink {
public:
    virtual ~ISharedAppEventSink() = default;

    virtual void publishEvent(const AppEvent &event) = 0;
};

class ISharedAppShellStateSource {
public:
    virtual ~ISharedAppShellStateSource() = default;

    virtual AppLaunchStage launchStage() const = 0;
    virtual AppLifecycleState lifecycleState() const = 0;
    virtual NavigationStateSnapshot navigationState() const = 0;
};
