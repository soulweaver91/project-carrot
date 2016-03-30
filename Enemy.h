#pragma once

#include <memory>
#include "CommonActor.h"
#include "CarrotQt5.h"
#include "EventMap.h"
#include <SFML/Graphics.hpp>

class Enemy : public CommonActor {
    public:
        Enemy(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0);
        ~Enemy();
        virtual void tickEvent() override;
        bool hurtsPlayer();

    protected:
        bool hurtPlayer;
        bool canMoveToPosition(double x, double y);
};

