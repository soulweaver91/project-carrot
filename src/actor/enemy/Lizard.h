#pragma once

#include <memory>

#include "Enemy.h"
#include "../../struct/Hitbox.h"

class CarrotQt5;

class EnemyLizard : public Enemy {
public:
    EnemyLizard(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0);
    ~EnemyLizard();
    void tickEvent() override;
    Hitbox getHitbox();
};
