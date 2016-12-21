#pragma once

#include "struct/Controls.h"
#include <SFML/Graphics.hpp>
#include "graphics/CarrotCanvas.h"
#include <memory>

class EngineState {
public:
    EngineState();
    ~EngineState();

    virtual void logicTick(const ControlEventList& events) = 0;
    virtual void renderTick(bool, bool) {};
    virtual void resizeEvent(int, int) {};
    virtual QString getType() = 0;
};