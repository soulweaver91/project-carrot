#pragma once

#include <memory>

#include "Enemy.h"
#include "../../struct/Hitbox.h"

class CarrotQt5;

class EnemyNormalTurtle : public Enemy {
public:
    EnemyNormalTurtle(std::shared_ptr<CarrotQt5>, double x = 0.0, double y = 0.0);
    ~EnemyNormalTurtle();
    void tickEvent() override;
    Hitbox getHitbox();
private:
    void onTransitionEndHook() override;
    bool isTurning;
    bool isWithdrawn;
};
