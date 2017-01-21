#pragma once

#include <memory>

#include "Ammo.h"
#include "../Player.h"

class CarrotQt5;

class AmmoBlaster : public Ammo {
public:
    AmmoBlaster(const ActorInstantiationDetails& initData, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double speed = 0.0, bool firedLeft = false, bool firedUp = false, bool poweredUp = false);
    ~AmmoBlaster();
    void tickEvent() override;
    WeaponType getType() const override;
};
