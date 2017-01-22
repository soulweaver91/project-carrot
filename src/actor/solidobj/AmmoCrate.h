#pragma once

#include "CrateContainer.h"
#include "../../struct/WeaponTypes.h"

class AmmoCrate : public CrateContainer {
public:
    AmmoCrate(const ActorInstantiationDetails& initData, WeaponType type);
    ~AmmoCrate();
};