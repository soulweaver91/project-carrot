#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

#include "SolidObject.h"
#include "../struct/PCEvent.h"

class ActorAPI;

class PushBox : public SolidObject {
public:
    PushBox(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0, int type = 0);
    ~PushBox();
};
