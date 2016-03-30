#pragma once

#include <memory>
#include "../Enemy.h"
#include "../CommonActor.h"
#include "../CarrotQt5.h"
#include <SFML/Graphics.hpp>

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
