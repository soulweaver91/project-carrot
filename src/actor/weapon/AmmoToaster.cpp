#include "AmmoToaster.h"
#include "../../gamestate/ActorAPI.h"
#include "../../gamestate/TileMap.h"

AmmoToaster::AmmoToaster(const ActorInstantiationDetails& initData, std::weak_ptr<Player> firedBy,
    double speed, bool firedLeft, bool firedUp, bool poweredUp)
    : Ammo(initData, firedBy, firedLeft, firedUp, 63, poweredUp) {
    isGravityAffected = false;
    loadResources("Weapon/Toaster");
    if (firedUp) {
        speedX = (qrand() % 100 - 50.0) / 100.0;
        speedY = (1.0 + qrand() % 100 * 0.001) * -3;
    } else {
        speedY = (qrand() % 100 - 50.0) / 100.0;
        speedX = (1.0 + qrand() % 100 * 0.001) * (firedLeft ? -1 : 1) + speed;
    }

    AnimationUser::setAnimation(poweredUp ? "WEAPON_TOASTER_POWERUP" : "WEAPON_TOASTER");
}


AmmoToaster::~AmmoToaster() {
}

void AmmoToaster::tickEvent() {
    Ammo::tickEvent();

    auto tiles = api->getGameTiles().lock();
    if (tiles == nullptr || tiles->isTileEmpty((posX + speedX) / 32, (posY + speedY) / 32)) {
        moveInstantly({ speedX, speedY }, false, true);
    } else {
        moveInstantly({ speedX, speedY }, false, true);
        checkCollisions();
        moveInstantly({ -speedX, -speedY }, false, true);

        if (!poweredUp) {
            health = 0;
        }
    }
}

WeaponType AmmoToaster::getType() const {
    return WEAPON_TOASTER;
}

void AmmoToaster::ricochet() {
    // do nothing
}
