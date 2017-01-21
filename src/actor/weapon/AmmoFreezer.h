#pragma once

#include <memory>

#include "Ammo.h"
#include "../Player.h"

class ActorAPI;

class AmmoFreezer : public Ammo {
public:
    AmmoFreezer(const ActorInstantiationDetails& initData, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double speed = 0.0, bool firedLeft = false, bool firedUp = false, bool poweredUp = false);
    ~AmmoFreezer();
    void tickEvent() override;
    int getFrozenDuration();
    WeaponType getType() const override;

private:
    void ricochet() override;
};
