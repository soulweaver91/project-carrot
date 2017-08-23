#pragma once

#include <memory>

#include "Ammo.h"
#include "../Player.h"

class ActorAPI;

class AmmoBouncer : public Ammo {
public:
    AmmoBouncer(const ActorInstantiationDetails& initData, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double initSpeed = 0.0, bool firedLeft = false, bool firedUp = false, bool poweredUp = false);
    ~AmmoBouncer();
    void tickEvent() override;
    WeaponType getType() const override;

private:
    void onHitFloorHook() override;
};
