#include "AmmoBlaster.h"
#include "../TriggerCrate.h"
#include "../../struct/Constants.h"
#include "../../gamestate/ActorAPI.h"

AmmoBlaster::AmmoBlaster(std::shared_ptr<ActorAPI> api, std::weak_ptr<Player> firedBy, double x, double y,
    double speed, bool firedLeft, bool firedUp)
    : Ammo(api, firedBy, x, y, firedLeft, firedUp, 24) {
    loadResources("Weapon/Blaster");
    if (firedUp) {
        speedY = -8;
        AnimationUser::setAnimation("WEAPON_BLASTER_VER");
    } else {
        AnimationUser::setAnimation("WEAPON_BLASTER_HOR");
        speedX = (firedLeft ? -12 : 12) + speed;
    }
}


AmmoBlaster::~AmmoBlaster() {
}

void AmmoBlaster::tickEvent() {
    Ammo::tickEvent();
    posX += speedX;
    posY += speedY;

    std::weak_ptr<SolidObject> actor;
    if (!api->isPositionEmpty(getHitbox(), false, shared_from_this(), actor)) {
        if (actor.lock() != nullptr && std::dynamic_pointer_cast<TriggerCrate>(actor.lock()) != nullptr) {
            ricochet();
        } else {
            health = 0;
            playSound("SFX_AMMO_HIT_WALL");

            auto animation = currentAnimation->getAnimation();
            double collisionX = posX + (speedY < -EPSILON ? 0 : speedX + (isFacingLeft ? -1 : 1) * animation->hotspot.x);
            double collisionY = posY + (speedY < -EPSILON ? speedY - animation->hotspot.y : 0);
            CoordinatePair c = {collisionX, collisionY};
            moveInstantly(c);
            checkCollisions();
        }
    }
}

WeaponType AmmoBlaster::getType() const {
    return WEAPON_BLASTER;
}
