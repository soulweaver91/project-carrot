#pragma once

#include "CommonActor.h"

class InteractiveActor : public CommonActor {
public:
    InteractiveActor(const ActorInstantiationDetails& initData, bool fromEventMap);
    virtual void processControlDownEvent(const ControlEvent& e);
    virtual void processControlUpEvent(const ControlEvent& e);
    virtual void processAllControlHeldEvents(const QMap<Control, ControlState>& e);

protected:
    void processAllControlHeldEventsDefaultHandler(const QMap<Control, ControlState>& e);
    virtual void processControlHeldEvent(const ControlEvent& e);
    ControlScheme controls;
    bool controllable;
};
