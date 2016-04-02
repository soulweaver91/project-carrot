#include "AmmoToaster.h"

Ammo_Toaster::Ammo_Toaster(std::shared_ptr<CarrotQt5> root, Player* firedBy, double x, double y, bool firedLeft, bool firedUp) 
    : Ammo(root, firedBy, x, y, firedLeft, firedUp, 70) {
    isGravityAffected = false;
    addAnimation(AnimState::IDLE,        "weapon/bullet_toaster.png",    9,1,18,26,9,10,24);
    if (firedUp) {
        speed_h = (qrand() % 100 - 50.0) / 100.0;
        speed_v = (1.0 + qrand() % 100 * 0.001) * -3;
    } else {
        speed_v = (qrand() % 100 - 50.0) / 100.0;
        speed_h = (1.0 + qrand() % 100 * 0.001) * (firedLeft ? -3 : 3);
    }
    setAnimation(AnimState::IDLE);
}


Ammo_Toaster::~Ammo_Toaster() {
}

void Ammo_Toaster::tickEvent() {
    Ammo::tickEvent();
    if (root->game_tiles->isTileEmpty((pos_x + speed_h) / 32, (pos_y + speed_v) / 32)) {
        pos_x += speed_h;
        pos_y += speed_v;
    } else {
        pos_x += speed_h;
        pos_y += speed_v;
        checkCollisions();
        pos_x -= speed_h;
        pos_y -= speed_v;

        if (!powered_up) {
            health = 0;
        }
    }
}

void Ammo_Toaster::ricochet() {
    // do nothing
}
