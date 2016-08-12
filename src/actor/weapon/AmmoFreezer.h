#pragma once

#include <memory>

#include "Ammo.h"
#include "../Player.h"

class CarrotQt5;

class AmmoFreezer : public Ammo {
public:
    AmmoFreezer(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double x = 0.0, double y = 0.0, double speed = 0.0, bool firedLeft = false, bool firedUp = false);
    ~AmmoFreezer();
    void tickEvent();
    int getFrozenDuration();
    WeaponType getType() const override;

private:
    void ricochet() override;
};
