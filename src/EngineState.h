#pragma once

#include "struct/Controls.h"

class EngineState {
public:
    EngineState();
    ~EngineState();

    virtual void logicTick(const ControlEventList& events) = 0;
    virtual void renderTick() {};
    virtual void resizeEvent(int, int) {};
};