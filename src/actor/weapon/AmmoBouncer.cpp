#include "AmmoBouncer.h"
#include <cmath>
#include "../../gamestate/ActorAPI.h"
#include "../../gamestate/TileMap.h"

AmmoBouncer::AmmoBouncer(std::shared_ptr<ActorAPI> api, std::weak_ptr<Player> firedBy, double x, double y,
    double speed, bool firedLeft, bool firedUp, bool poweredUp)
    : Ammo(api, firedBy, x, y, firedLeft, firedUp, 140, poweredUp) {
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
    auto tiles = api->getGameTiles().lock();
    if (tiles == nullptr || tiles->isTileEmpty((posX + speedX) / 32, (posY + speedY) / 32)) {
        moveInstantly({ speedX, speedY }, false, true);
    } else {
        CoordinatePair temp = {posX, posY};
        CoordinatePair next = {posX + speedX, posY + speedY};
        moveInstantly(next, true, true);
        checkCollisions();
        moveInstantly(temp, true, true);
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
