#pragma once

#include <memory>

#include "Ammo.h"
#include "../Player.h"

class CarrotQt5;

class AmmoBlaster : public Ammo {
public:
    AmmoBlaster(std::shared_ptr<ActorAPI> api, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double x = 0.0, double y = 0.0, double speed = 0.0, bool firedLeft = false, bool firedUp = false, bool poweredUp = false);
    ~AmmoBlaster();
    void tickEvent();
    WeaponType getType() const override;
};
