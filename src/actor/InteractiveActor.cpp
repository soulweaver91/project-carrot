#include "InteractiveActor.h"

InteractiveActor::InteractiveActor(const ActorInstantiationDetails& initData, bool fromEventMap)
    : CommonActor(initData, fromEventMap), controllable(true) {
}

void InteractiveActor::processControlDownEvent(const ControlEvent&) {
    // nothing to do in this event unless a child class
    // overrides the function
}

void InteractiveActor::processControlUpEvent(const ControlEvent&) {
    // nothing to do in this event unless a child class
    // overrides the function
}

void InteractiveActor::processControlHeldEvent(const ControlEvent&) {
    // nothing to do in this event unless a child class
    // overrides the function
}

void InteractiveActor::processAllControlHeldEvents(const QMap<Control, ControlState>&) {
    // By default, there is no functionality here. Most actors do not process control
    // events at all. If they do, they can override this function and call the
    // processAllControlHeldEventsDefaultHandler function for easy handling of each
    // key directly in processControlHeldEvent. Keys that do one thing if they are
    // held and another if not should not be handled there but here directly, though.
}

void InteractiveActor::processAllControlHeldEventsDefaultHandler(const QMap<Control, ControlState>& e) {
    // By default, just go through all events.
    for (const auto& ev : e.keys()) {
        processControlHeldEvent(qMakePair(ev, e.value(ev)));
    }
}
