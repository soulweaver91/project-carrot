#include "AmmoBlaster.h"
#include "../TriggerCrate.h"

AmmoBlaster::AmmoBlaster(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> firedBy, double x, double y, bool firedLeft, bool firedUp)
    : Ammo(root, firedBy, x, y, firedLeft, firedUp, 24) {
    loadResources("Weapon/Blaster");
    if (firedUp) {
        speedY = -8;
        AnimationUser::setAnimation("WEAPON_BLASTER_VER");
    } else {
        AnimationUser::setAnimation("WEAPON_BLASTER_HOR");
        speedX = (firedLeft ? -12 : 12);
    }
}


AmmoBlaster::~AmmoBlaster() {
}

void AmmoBlaster::tickEvent() {
    Ammo::tickEvent();
    posX += speedX;
    posY += speedY;

    std::weak_ptr<SolidObject> actor;
    if (!root->isPositionEmpty(getHitbox(), false, shared_from_this(), actor)) {
        if (actor.lock() != nullptr && std::dynamic_pointer_cast<TriggerCrate>(actor.lock()) != nullptr) {
            ricochet();
        } else {
            health = 0;
            playSound("SFX_AMMO_HIT_WALL");

            double collisionX = posX + (speedY < -1e-6 ? 0 : speedX + (isFacingLeft ? -1 : 1) * currentAnimation.animation->hotspot.x);
            double collisionY = posY + (speedY < -1e-6 ? speedY - currentAnimation.animation->hotspot.y : 0);
            CoordinatePair c = {collisionX, collisionY};
            moveInstantly(c);
            checkCollisions();
        }
    }
}
