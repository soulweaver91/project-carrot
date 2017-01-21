#pragma once

#include <memory>

#include "Enemy.h"
#include "../../struct/Hitbox.h"

class ActorAPI;

class EnemyLizard : public Enemy {
public:
    EnemyLizard(const ActorInstantiationDetails& initData);
    ~EnemyLizard();
    void tickEvent() override;
    void updateHitbox() override;
};
