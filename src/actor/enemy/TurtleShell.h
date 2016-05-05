#pragma once

#include "../CommonActor.h"

class TurtleShell : public CommonActor {
public:
    TurtleShell(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0, double initSpeedX = 0.0,
        double initSpeedY = 0.0, bool fromEventMap = false);
    ~TurtleShell();
    void tickEvent() override;
    void impact(double speed);
    Hitbox getHitbox() override;

protected:
    void onHitFloorHook() override;
};
