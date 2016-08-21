#pragma once

#include <memory>

#include "Enemy.h"

class ActorAPI;

class EnemySuckerFloat : public Enemy {
public:
    EnemySuckerFloat(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0);
    ~EnemySuckerFloat();
    void tickEvent() override;
    bool perish() override;

private:
    double phase;
    double originX;
    double originY;
};
