#pragma once

#include <memory>

#include "Enemy.h"
#include "../../struct/Hitbox.h"

class CarrotQt5;

class Enemy_NormalTurtle : public Enemy {
    public:
        Enemy_NormalTurtle(std::shared_ptr<CarrotQt5>, double x = 0.0, double y = 0.0);
        ~Enemy_NormalTurtle();
        void tickEvent() override;
        Hitbox getHitbox();
    private:
        void onTransitionEndHook() override;
        bool isTurning;
        bool isWithdrawn;
};
