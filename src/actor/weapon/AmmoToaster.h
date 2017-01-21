#pragma once

#include <memory>

#include "Ammo.h"
#include "../Player.h"

class ActorAPI;

class AmmoToaster : public Ammo {
public:
    AmmoToaster(const ActorInstantiationDetails& initData, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double speed = 0.0, bool firedLeft = false, bool firedUp = false, bool poweredUp = false);
    ~AmmoToaster();
    void tickEvent() override;
    WeaponType getType() const override;

private:
    void ricochet() override;
};
