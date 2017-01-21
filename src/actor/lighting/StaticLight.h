#pragma once

#include <memory>
#include "../CommonActor.h"
#include "RadialLightSource.h"

class StaticLight : public CommonActor, public RadialLightSource {
public:
    StaticLight(const ActorInstantiationDetails& initData, quint16 alpha);
    ~StaticLight();
};
