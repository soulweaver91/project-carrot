#pragma once

#include "../Enemy.h"
#include "../CommonActor.h"
#include "../CarrotQt5.h"
#include <SFML/Graphics.hpp>

class Enemy_NormalTurtle : public Enemy {
    public:
        Enemy_NormalTurtle(CarrotQt5* root, double x = 0.0, double y = 0.0);
        ~Enemy_NormalTurtle();
        void tickEvent() override;
        Hitbox getHitbox();
    private:
        void onTransitionEndHook() override;
        bool isTurning;
        bool isWithdrawn;
};
