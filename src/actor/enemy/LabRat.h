#pragma once

#include <memory>

#include "Enemy.h"
#include "../../struct/Hitbox.h"

class ActorAPI;

class EnemyLabRat : public Enemy {
public:
    EnemyLabRat(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0);
    ~EnemyLabRat();
    void tickEvent() override;
    void attack();

private:
    bool canAttack;
    bool idling;
    bool canIdle;
};
