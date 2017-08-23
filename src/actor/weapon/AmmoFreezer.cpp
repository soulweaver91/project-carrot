#include "AmmoFreezer.h"
#include "../../gamestate/ActorAPI.h"
#include "../../gamestate/TileMap.h"

AmmoFreezer::AmmoFreezer(const ActorInstantiationDetails& initData, std::weak_ptr<Player> firedBy,
    double initSpeed, bool firedLeft, bool firedUp, bool poweredUp)
    : Ammo(initData, firedBy, firedLeft, firedUp, 70, poweredUp) {
    isGravityAffected = false;
    strength = 0;
    loadResources("Weapon/Freezer");
    if (firedUp) {
        speed.y = -4;
        AnimationUser::setAnimation("WEAPON_FREEZER_VER");
    } else {
        speed.x = 8 * (firedLeft ? -1 : 1) + initSpeed;
        AnimationUser::setAnimation("WEAPON_FREEZER_HOR");
    }
}


AmmoFreezer::~AmmoFreezer() {

}

void AmmoFreezer::tickEvent() {
    Ammo::tickEvent();

    if (api->isTileCollisionFree((pos + speed).tilePosition())) {
        moveInstantly(speed, false, true);
    } else {
        moveInstantly(speed, false, true);
        checkCollisions();
        moveInstantly(-speed, false, true);
        health = 0;
    }
}

int AmmoFreezer::getFrozenDuration() {
    return 70 * 5;
}

WeaponType AmmoFreezer::getType() const {
    return WEAPON_FREEZER;
}

void AmmoFreezer::ricochet() {
    // do nothing
}
