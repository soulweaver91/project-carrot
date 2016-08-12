#include "AmmoBouncer.h"
#include "../../CarrotQt5.h"
#include "../../gamestate/TileMap.h"

AmmoBouncer::AmmoBouncer(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> firedBy, double x, double y,
    double speed, bool firedLeft, bool firedUp)
    : Ammo(root, firedBy, x, y, firedLeft, firedUp, 140) {
    elasticity = 0.9;
    loadResources("Weapon/Bouncer");
    if (firedUp) {
        speedY = -2;
        isGravityAffected = false;
    } else {
        speedX = (firedLeft ? -3 : 3) + speed;
    }
    setAnimation(AnimState::IDLE);
}


AmmoBouncer::~AmmoBouncer() {
}

void AmmoBouncer::tickEvent() {
    CommonActor::tickEvent();
    Ammo::tickEvent();
    auto tiles = root->getGameTiles().lock();
    if (tiles == nullptr || tiles->isTileEmpty((posX + speedX) / 32, (posY + speedY) / 32)) {
        posX += speedX;
        posY += speedY;
    } else {
        CoordinatePair temp = {posX, posY};
        CoordinatePair next = {posX + speedX, posY + speedY};
        moveInstantly(next);
        checkCollisions();
        moveInstantly(temp);
    }
}

WeaponType AmmoBouncer::getType() const {
    return WEAPON_BOUNCER;
}

void AmmoBouncer::onHitFloorHook() {
    if (speedY < 0) {
        speedY = std::min(std::max(-4.0, speedY), -1.0);
    }
}
