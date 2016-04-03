#pragma once

#include <memory>

#include "Ammo.h"
#include "../Player.h"

class CarrotQt5;

class Ammo_Blaster : public Ammo {
public:
    Ammo_Blaster(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double x = 0.0, double y = 0.0, bool firedLeft = false, bool firedUp = false);
    ~Ammo_Blaster();
    void tickEvent();
};
