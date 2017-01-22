#pragma once

#include "BarrelContainer.h"
#include "AmmoContainer.h"
#include "../../struct/WeaponTypes.h"

class AmmoBarrel : public BarrelContainer, public AmmoContainer {
public:
    AmmoBarrel(const ActorInstantiationDetails& initData, WeaponType type);
    ~AmmoBarrel();
    void handleCollision(std::shared_ptr<CommonActor> other) override;
};