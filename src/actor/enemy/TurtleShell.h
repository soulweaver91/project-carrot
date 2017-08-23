#pragma once

#include "../CommonActor.h"

class ActorAPI;

class TurtleShell : public CommonActor {
public:
    TurtleShell(const ActorInstantiationDetails& initData, sf::Vector2f initSpeed = { 0.0, 0.0 });
    ~TurtleShell();
    void tickEvent() override;
    void updateHitbox() override;
    virtual void handleCollision(std::shared_ptr<CommonActor> other) override;

protected:
    void onHitFloorHook() override;
};
