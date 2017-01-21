#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

#include "SolidObject.h"
#include "../struct/PCEvent.h"

class ActorAPI;

class PushBox : public SolidObject {
public:
    PushBox(const ActorInstantiationDetails& initData, int type = 0);
    ~PushBox();
};
