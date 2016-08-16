#pragma once

#include "struct/Controls.h"

class ModeManager {
public:
    ModeManager();
    ~ModeManager();
    virtual void tick(const ControlEventList& events) = 0;
};