#pragma once

#include <memory>

#include "Enemy.h"

class ActorAPI;

class EnemySucker : public Enemy {
public:
    EnemySucker(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0, LastHitDirection dir = NONE);
    ~EnemySucker();
    void tickEvent() override;

private:
    int cycle;
};
