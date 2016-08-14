#pragma once

#include "CommonActor.h"

class InteractiveActor : public CommonActor {
public:
    InteractiveActor(std::shared_ptr<CarrotQt5> gameRoot, double x, double y, bool fromEventMap);
    virtual void processControlDownEvent(const ControlEvent& e);
    virtual void processControlUpEvent(const ControlEvent& e);
    virtual void processAllControlHeldEvents(const QMap<Control, ControlState>& e);

protected:
    void processAllControlHeldEventsDefaultHandler(const QMap<Control, ControlState>& e);
    virtual void processControlHeldEvent(const ControlEvent& e);
    ControlScheme controls;
    bool controllable;
};
