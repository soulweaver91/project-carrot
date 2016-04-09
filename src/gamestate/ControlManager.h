#pragma once
#include <QMap>
#include <QVector>
#include "../struct/Controls.h"

class ControlManager {
public:
    bool isControlPressed(const Control& control);
    bool isControlPressed(const int& key);
    void setControlHeldDown(const Control& control);
    void setControlHeldDown(const int& key);
    void setControlReleased(const Control& control);
    void setControlReleased(const int& key);
    void processFrame();
    ControlEventList getPendingEvents();

private:
    QMap<Control, ControlState> controlStates;
    static const ControlState stateTemplate;
};