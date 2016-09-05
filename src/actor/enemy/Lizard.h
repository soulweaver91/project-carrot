#pragma once

#include <memory>

#include "Enemy.h"
#include "../../struct/Hitbox.h"

class ActorAPI;

class EnemyLizard : public Enemy {
public:
    EnemyLizard(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0);
    ~EnemyLizard();
    void tickEvent() override;
    void updateHitbox() override;
};
