#pragma once

#include <memory>

#include "Enemy.h"

class ActorAPI;

class EnemySucker : public Enemy {
public:
    EnemySucker(const ActorInstantiationDetails& initData, LastHitDirection dir = NONE);
    ~EnemySucker();
    void tickEvent() override;

private:
    int cycle;
};
