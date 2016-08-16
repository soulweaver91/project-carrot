#pragma once

#include <memory>

#include "CommonActor.h"

class ActorAPI;

class SavePoint : public CommonActor {
public:
    SavePoint(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0);
    ~SavePoint();
    void tickEvent();
    void activateSavePoint();
    Hitbox getHitbox() override;

private:
    bool activated;
};
