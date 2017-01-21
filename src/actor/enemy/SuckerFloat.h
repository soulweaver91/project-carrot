#pragma once

#include <memory>

#include "Enemy.h"

class ActorAPI;

class EnemySuckerFloat : public Enemy {
public:
    EnemySuckerFloat(const ActorInstantiationDetails& initData);
    ~EnemySuckerFloat();
    void tickEvent() override;
    bool perish() override;

private:
    double phase;
    double originX;
    double originY;
};
