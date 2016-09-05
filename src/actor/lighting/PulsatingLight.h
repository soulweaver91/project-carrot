#pragma once

#include <memory>
#include "../CommonActor.h"
#include "RadialLightSource.h"

class PulsatingLight : public CommonActor, public RadialLightSource {
public:
    PulsatingLight(std::shared_ptr<ActorAPI> api, double x, double y, quint16 alpha, quint16 speed, quint16 sync);
    ~PulsatingLight();
    void tickEvent() override;

private:
    double phase;
    double speed;
};
