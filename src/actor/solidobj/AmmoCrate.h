#pragma once

#include "CrateContainer.h"
#include "AmmoContainer.h"
#include "../../struct/WeaponTypes.h"

class AmmoCrate : public CrateContainer, public AmmoContainer {
public:
    AmmoCrate(const ActorInstantiationDetails& initData, WeaponType type);
    ~AmmoCrate();
    void handleCollision(std::shared_ptr<CommonActor> other) override;

protected:
    PCEvent eventFromType(WeaponType type);
};