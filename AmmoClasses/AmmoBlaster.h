#ifndef H_CP_AMMO_BLASTER
#define H_CP_AMMO_BLASTER

#include "CommonActor.h"
#include "CarrotQt5.h"
#include "Player.h"
#include "WeaponTypes.h"
#include "Ammo.h"

class Ammo_Blaster : public Ammo {
public:
    Ammo_Blaster(CarrotQt5* root, Player* firedBy = nullptr, double x = 0.0, double y = 0.0, bool firedLeft = false, bool firedUp = false);
    ~Ammo_Blaster();
    void tickEvent();
};

#endif
