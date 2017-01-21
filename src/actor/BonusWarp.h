#pragma once

#include "CommonActor.h"
#include "../struct/CoordinatePair.h"

class ActorAPI;

class BonusWarp : public CommonActor {
public:
    BonusWarp(const ActorInstantiationDetails& initData, const quint16 params[8]);
    ~BonusWarp();
    const CoordinatePair getWarpTarget();
    quint16 getCost();

private:
    quint16 eventParams[8];
};
