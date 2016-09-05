#include "AmmoFreezer.h"
#include "../../gamestate/ActorAPI.h"
#include "../../gamestate/TileMap.h"

AmmoFreezer::AmmoFreezer(std::shared_ptr<ActorAPI> api, std::weak_ptr<Player> firedBy, double x, double y,
    double speed, bool firedLeft, bool firedUp, bool poweredUp)
    : Ammo(api, firedBy, x, y, firedLeft, firedUp, 70, poweredUp) {
    isGravityAffected = false;
    strength = 0;
    loadResources("Weapon/Freezer");
    if (firedUp) {
        speedY = -4;
        AnimationUser::setAnimation("WEAPON_FREEZER_VER");
    } else {
        speedX = 8 * (firedLeft ? -1 : 1) + speed;
        AnimationUser::setAnimation("WEAPON_FREEZER_HOR");
    }
}


AmmoFreezer::~AmmoFreezer() {

}

void AmmoFreezer::tickEvent() {
    Ammo::tickEvent();

    auto tiles = api->getGameTiles().lock();
    if (tiles == nullptr || tiles->isTileEmpty((posX + speedX) / 32, (posY + speedY) / 32)) {
        moveInstantly({ speedX, speedY }, false);
    } else {
        moveInstantly({ speedX, speedY }, false);
        checkCollisions();
        moveInstantly({ -speedX, -speedY }, false);
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
