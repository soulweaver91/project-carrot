#include "AmmoToaster.h"
#include "../../CarrotQt5.h"
#include "../../gamestate/TileMap.h"

AmmoToaster::AmmoToaster(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> firedBy, double x, double y, bool firedLeft, bool firedUp)
    : Ammo(root, firedBy, x, y, firedLeft, firedUp, 70) {
    isGravityAffected = false;
    loadResources("Weapon/Toaster");
    if (firedUp) {
        speedX = (qrand() % 100 - 50.0) / 100.0;
        speedY = (1.0 + qrand() % 100 * 0.001) * -3;
    } else {
        speedY = (qrand() % 100 - 50.0) / 100.0;
        speedX = (1.0 + qrand() % 100 * 0.001) * (firedLeft ? -3 : 3);
    }
    setAnimation(AnimState::IDLE);
}


AmmoToaster::~AmmoToaster() {
}

void AmmoToaster::tickEvent() {
    Ammo::tickEvent();

    auto tiles = root->getGameTiles().lock();
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

void AmmoToaster::ricochet() {
    // do nothing
}
