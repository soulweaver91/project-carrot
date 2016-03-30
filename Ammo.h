#pragma once

#include "CommonActor.h"
#include "CarrotQt5.h"
#include "Player.h"
#include "WeaponTypes.h"

class Ammo : public CommonActor {
public:
    Ammo(CarrotQt5* root, Player* firedBy = nullptr, double x = 0.0, double y = 0.0, bool firedLeft = false, bool firedUp = false, int alive = 70, bool powered = false);
    ~Ammo();
    void tickEvent();

protected:
    void checkCollisions();
    bool powered_up;
    virtual void ricochet();
    
private:
    Player* owner;
    double start_x;
    double start_y;
    int ttl;
};
