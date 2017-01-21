#pragma once

#include <memory>

#include "CommonActor.h"

class ActorAPI;

class SavePoint : public CommonActor {
public:
    SavePoint(const ActorInstantiationDetails& initData);
    ~SavePoint();
    void tickEvent() override;
    void activateSavePoint();
    void updateHitbox() override;

private:
    bool activated;
};
