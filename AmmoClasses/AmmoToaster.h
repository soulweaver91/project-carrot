#ifndef H_CP_AMMO_TOASTER
#define H_CP_AMMO_TOASTER

#include "CommonActor.h"
#include "CarrotQt5.h"
#include "Player.h"
#include "WeaponTypes.h"
#include "Ammo.h"

class Ammo_Toaster : public Ammo {
    public:
        Ammo_Toaster(CarrotQt5* root, Player* firedBy = nullptr, double x = 0.0, double y = 0.0, bool firedLeft = false, bool firedUp = false);
        ~Ammo_Toaster();
        void tickEvent();

    private:
        void Ammo_Toaster::ricochet();
};

#endif
