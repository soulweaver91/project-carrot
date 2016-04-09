#pragma once

#include <QVector>
#include <QPair>
#include <QMap>

// TODO: Add joystick controls here as well
// Probably use QGamepad from Qt 5.7?
struct Control {
    int keyboardKey;
    bool isKeyboard;

    // Required by QMap; returns an invalid control,
    // so should not be used in any self-written code
    Control::Control() {
        keyboardKey = 0;
        isKeyboard = false;
    }

    Control::Control(const int& key) {
        keyboardKey = key;
        isKeyboard = true;
    }

    Control::Control(const Control& other) {
        keyboardKey = other.keyboardKey;
        isKeyboard = other.isKeyboard;
    }

    bool operator<(const Control& other) const {
        if (isKeyboard && !other.isKeyboard) {
            return true;
        }

        return keyboardKey < other.keyboardKey;
    }

    bool operator==(const int& key) const {
        return isKeyboard && keyboardKey == key;
    }

    bool operator==(const Control& other) const {
        return other.isKeyboard == isKeyboard && other.keyboardKey == keyboardKey;
    }
};

struct ControlState {
    unsigned long heldDuration;
    bool pressStarted;
    bool pressEnded;
};

typedef QPair<Control, ControlState> ControlEvent;

struct ControlEventList {
    QVector<ControlEvent> controlDownEvents;
    QVector<ControlEvent> controlUpEvents;
    QMap<Control, ControlState> controlHeldEvents;
};
