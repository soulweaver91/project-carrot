#include "AmmoToaster.h"
#include "../../gamestate/ActorAPI.h"
#include "../../gamestate/TileMap.h"

AmmoToaster::AmmoToaster(std::shared_ptr<ActorAPI> api, std::weak_ptr<Player> firedBy, double x, double y,
    double speed, bool firedLeft, bool firedUp, bool poweredUp)
    : Ammo(api, firedBy, x, y, firedLeft, firedUp, 63, poweredUp) {
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
        posX += speedX;
        posY += speedY;
    } else {
        posX += speedX;
        posY += speedY;
        checkCollisions();
        posX -= speedX;
        posY -= speedY;

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
