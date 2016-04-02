#include "AmmoBlaster.h"
#include "../TriggerCrate.h"

Ammo_Blaster::Ammo_Blaster(std::shared_ptr<CarrotQt5> root, Player* firedBy, double x, double y, bool firedLeft, bool firedUp)
    : Ammo(root, firedBy, x, y, firedLeft, firedUp, 24) {
    if (firedUp) {
        speed_v = -8;
        addAnimation(AnimState::IDLE,        "weapon/bullet_blaster_ver.png",    3,1,4,13,10,2,7);
    } else {
        speed_h = (firedLeft ? -12 : 12);
        addAnimation(AnimState::IDLE,        "weapon/bullet_blaster_hor.png",    3,1,13,4,10,7,2);
    }
    setAnimation(AnimState::IDLE);
}


Ammo_Blaster::~Ammo_Blaster() {
}

void Ammo_Blaster::tickEvent() {
    Ammo::tickEvent();
    pos_x += speed_h;
    pos_y += speed_v;

    std::weak_ptr<SolidObject> obj;
    if (!root->isPositionEmpty(getHitbox(), false, shared_from_this(), obj)) {
        if (obj.lock() != nullptr && std::dynamic_pointer_cast<TriggerCrate>(obj.lock()) != nullptr) {
            ricochet();
        } else {
            health = 0;
            root->sfxsys->playSFX(SFX_AMMO_HIT_WALL);

            double coll_x = pos_x + (speed_v < -1e-6 ? 0 : speed_h + (facingLeft ? -1 : 1) * current_animation->offset_x);
            double coll_y = pos_y + (speed_v < -1e-6 ? speed_v - current_animation->offset_y : 0);
            CoordinatePair c = {coll_x, coll_y};
            moveInstantly(c);
            checkCollisions();
        }
    }
}
