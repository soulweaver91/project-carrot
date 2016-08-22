#pragma once

#include <memory>

#include "Ammo.h"
#include "../Player.h"

class ActorAPI;

class AmmoToaster : public Ammo {
public:
    AmmoToaster(std::shared_ptr<ActorAPI> api, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double x = 0.0, double y = 0.0, double speed = 0.0, bool firedLeft = false, bool firedUp = false, bool poweredUp = false);
    ~AmmoToaster();
    void tickEvent();
    WeaponType getType() const override;

private:
    void ricochet() override;
};
