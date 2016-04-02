#pragma once

#include <memory>

#include "../../CarrotQt5.h"
#include "../CommonActor.h"

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

