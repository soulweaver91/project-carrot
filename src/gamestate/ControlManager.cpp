#include "ControlManager.h"

const ControlState ControlManager::stateTemplate = {
    false,
    0,
    false
};

bool ControlManager::isControlPressed(const int& key) {
    return isControlPressed(Control(key));
}

bool ControlManager::isControlPressed(const Control& control) {
    return controlStates.contains(control);
}

void ControlManager::setControlHeldDown(const int& key) {
    setControlHeldDown(Control(key));
}

void ControlManager::setControlHeldDown(const Control& control) {
    if (!controlStates.contains(control)) {
        ControlState state(stateTemplate);
        state.pressStarted = true;
        controlStates.insert(control, state);
    }
}

void ControlManager::setControlReleased(const int& key) {
    setControlReleased(Control(key));
}

void ControlManager::setControlReleased(const Control& control) {
    if (!controlStates.contains(control)) {
        ControlState state(stateTemplate);
        controlStates.insert(control, state);
    }

    ControlState& state = controlStates[control];
    state.pressEnded = true;
}

void ControlManager::processFrame() {
    for (const auto& control : controlStates.keys()) {
        ControlState& state = controlStates[control];

        if (state.pressStarted) {
            state.pressStarted = false;
        }

        if (state.pressEnded) {
            controlStates.remove(control);
            continue;
        }

        state.heldDuration++;
    }
}


ControlEventList ControlManager::getPendingEvents() {
    ControlEventList events;
    for (const auto& control : controlStates.keys()) {
        const ControlState& state = controlStates.value(control);
        auto event = qMakePair(control, state);

        if (state.pressStarted) {
            events.controlDownEvents << event;
        }

        if (state.pressEnded) {
            events.controlUpEvents << event;
        }

        // There can be a down and an up event on the same frame,
        // so the control can produce both of the above. However, a
        // held event is only wanted if neither of those is true.
        // Example: (U = up, D = down, H = held event)
        // 0 frames between up and down = U+D on frame 1
        // 1 frame  = U on frame 1, D on frame 2
        // 2 frames = U on frame 1, H on 2, D on 3
        // and so on.
        if (!state.pressEnded && !state.pressStarted) {
            events.controlHeldEvents.insert(event.first, event.second);
        }
    }

    return events;
}

void ControlManager::releaseAllKeys() {
    for (auto control : controlStates.keys()) {
        setControlReleased(control);
    }
}
