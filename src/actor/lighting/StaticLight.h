#pragma once

#include <memory>
#include "../CommonActor.h"
#include "RadialLightSource.h"

class StaticLight : public CommonActor, public RadialLightSource {
public:
    StaticLight(std::shared_ptr<ActorAPI> api, double x, double y, quint16 alpha);
    ~StaticLight();
};
