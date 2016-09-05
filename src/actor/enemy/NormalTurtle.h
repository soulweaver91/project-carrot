#pragma once

#include <memory>

#include "Enemy.h"
#include "../../struct/Hitbox.h"

class ActorAPI;

class EnemyNormalTurtle : public Enemy {
public:
    EnemyNormalTurtle(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0);
    ~EnemyNormalTurtle();
    void tickEvent() override;
    void updateHitbox() override;
    bool perish() override;

private:
    void attack();
    void handleTurn(bool isFirstPhase);
    bool isTurning;
    bool isWithdrawn;
};
