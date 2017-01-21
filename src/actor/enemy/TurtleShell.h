#pragma once

#include "../CommonActor.h"

class ActorAPI;

class TurtleShell : public CommonActor {
public:
    TurtleShell(const ActorInstantiationDetails& initData, double initSpeedX = 0.0,
        double initSpeedY = 0.0, bool fromEventMap = false);
    ~TurtleShell();
    void tickEvent() override;
    void updateHitbox() override;
    virtual void handleCollision(std::shared_ptr<CommonActor> other) override;

protected:
    void onHitFloorHook() override;
};
