#include "AmmoBlaster.h"
#include "../solidobj/TriggerCrate.h"
#include "../solidobj/BarrelContainer.h"
#include "../../struct/Constants.h"
#include "../../gamestate/ActorAPI.h"

AmmoBlaster::AmmoBlaster(const ActorInstantiationDetails& initData, std::weak_ptr<Player> firedBy,
    double initSpeed, bool firedLeft, bool firedUp, bool poweredUp)
    : Ammo(initData, firedBy, firedLeft, firedUp, 24, poweredUp) {
    loadResources("Weapon/Blaster");
    if (firedUp) {
        speed.y = -8;
        AnimationUser::setAnimation("WEAPON_BLASTER_VER");
    } else {
        AnimationUser::setAnimation("WEAPON_BLASTER_HOR");
        speed.x = (firedLeft ? -12 : 12) + initSpeed;
    }
}


AmmoBlaster::~AmmoBlaster() {
}

void AmmoBlaster::tickEvent() {
    Ammo::tickEvent();
    moveInstantly(speed, false, true);

    std::weak_ptr<SolidObject> actor;
    if (!api->isPositionEmpty(currentHitbox, false, shared_from_this(), actor)) {
        if (actor.lock() != nullptr
            && std::dynamic_pointer_cast<TriggerCrate>(actor.lock()) != nullptr
            && std::dynamic_pointer_cast<BarrelContainer>(actor.lock()) != nullptr) {
            ricochet();
        } else {
            health = 0;
            playSound("SFX_AMMO_HIT_WALL");

            auto animation = currentAnimation->getAnimation();
            double collisionX = pos.x + (speed.y < -EPSILON ? 0 : speed.x + (isFacingLeft ? -1 : 1) * animation->hotspot.x);
            double collisionY = pos.y + (speed.y < -EPSILON ? speed.y - animation->hotspot.y : 0);
            CoordinatePair c = {collisionX, collisionY};
            moveInstantly(c, true);
            checkCollisions();
        }
    }
}

WeaponType AmmoBlaster::getType() const {
    return WEAPON_BLASTER;
}
