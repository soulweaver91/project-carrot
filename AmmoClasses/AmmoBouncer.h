#ifndef H_CP_AMMO_BOUNCER
#define H_CP_AMMO_BOUNCER

#include "CommonActor.h"
#include "CarrotQt5.h"
#include "Player.h"
#include "WeaponTypes.h"
#include "Ammo.h"

class Ammo_Bouncer : public Ammo {
public:
    Ammo_Bouncer(CarrotQt5* root, Player* firedBy = nullptr, double x = 0.0, double y = 0.0, bool firedLeft = false, bool firedUp = false);
    ~Ammo_Bouncer();
    void tickEvent();
private:
    void Ammo_Bouncer::onHitFloorHook();
};

#endif
