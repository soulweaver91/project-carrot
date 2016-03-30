#include <memory>
#include "CommonActor.h"
#include "CarrotQt5.h"
#include "AmmoBouncer.h"
#include "Ammo.h"

Ammo_Bouncer::Ammo_Bouncer(std::shared_ptr<CarrotQt5> root, Player* firedBy, double x, double y, bool firedLeft, bool firedUp)
    : Ammo(root, firedBy, x, y, firedLeft, firedUp, 140) {
    elasticity = 0.9;
    addAnimation(AnimState::IDLE,        "weapon/bullet_bouncer.png",    8,1,10,7,10,5,4);
    if (firedUp) {
        speed_v = -2;
        isGravityAffected = false;
    } else {
        speed_h = (firedLeft ? -3 : 3);
    }
    setAnimation(AnimState::IDLE);
}


Ammo_Bouncer::~Ammo_Bouncer() {
}

void Ammo_Bouncer::tickEvent() {
    CommonActor::tickEvent();
    Ammo::tickEvent();
    if (root->game_tiles->isTileEmpty((pos_x + speed_h) / 32, (pos_y + speed_v) / 32)) {
        pos_x += speed_h;
        pos_y += speed_v;
    } else {
        CoordinatePair temp = {pos_x, pos_y};
        CoordinatePair next = {pos_x + speed_h, pos_y + speed_v};
        moveInstantly(next);
        checkCollisions();
        moveInstantly(temp);
    }
}

void Ammo_Bouncer::onHitFloorHook() {
    if (speed_v < 0) {
        speed_v = std::min(std::max(-4.0,speed_v),-1.0);
    }
}
