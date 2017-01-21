#pragma once

#include <memory>

#include "Enemy.h"
#include "../../struct/Hitbox.h"

class ActorAPI;

class EnemyLabRat : public Enemy {
public:
    EnemyLabRat(const ActorInstantiationDetails& initData);
    ~EnemyLabRat();
    void tickEvent() override;
    void attack();

private:
    bool canAttack;
    bool idling;
    bool canIdle;
};
