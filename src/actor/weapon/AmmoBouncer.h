#pragma once

#include <memory>

#include "../../CarrotQt5.h"
#include "Ammo.h"
#include "../Player.h"

class Ammo_Bouncer : public Ammo {
public:
    Ammo_Bouncer(std::shared_ptr<CarrotQt5> root, Player* firedBy = nullptr, double x = 0.0, double y = 0.0, 
        bool firedLeft = false, bool firedUp = false);
    ~Ammo_Bouncer();
    void tickEvent();
private:
    void Ammo_Bouncer::onHitFloorHook();
};
