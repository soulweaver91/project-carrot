#pragma once

#include <memory>

#include "../../CarrotQt5.h"
#include "Enemy.h"

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
