#pragma once

#include "CommonActor.h"
#include "../struct/CoordinatePair.h"

class ActorAPI;

class BonusWarp : public CommonActor {
public:
    BonusWarp(std::shared_ptr<ActorAPI> api, double x, double y, const quint16 params[8]);
    ~BonusWarp();
    const CoordinatePair getWarpTarget();
    quint16 getCost();

private:
    quint16 eventParams[8];
};
