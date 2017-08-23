#include "AmmoBouncer.h"
#include <cmath>
#include "../../gamestate/ActorAPI.h"
#include "../../gamestate/TileMap.h"

AmmoBouncer::AmmoBouncer(const ActorInstantiationDetails& initData, std::weak_ptr<Player> firedBy,
    double initSpeed, bool firedLeft, bool firedUp, bool poweredUp)
    : Ammo(initData, firedBy, firedLeft, firedUp, 140, poweredUp) {
    elasticity = 0.9;
    loadResources("Weapon/Bouncer");
    if (firedUp) {
        speed.y = -2;
        isGravityAffected = false;
    } else {
        speed.x = (firedLeft ? -3 : 3) + initSpeed;
    }
    setAnimation(AnimState::IDLE);
}


AmmoBouncer::~AmmoBouncer() {
}

void AmmoBouncer::tickEvent() {
    CommonActor::tickEvent();
    Ammo::tickEvent();
    auto tiles = api->getGameTiles().lock();
    if (tiles == nullptr || tiles->isTileEmpty((pos + speed).tilePosition())) {
        moveInstantly(speed, false, true);
    } else {
        CoordinatePair temp = pos;
        CoordinatePair next = pos + speed;
        moveInstantly(next, true, true);
        checkCollisions();
        moveInstantly(temp, true, true);
    }
}

WeaponType AmmoBouncer::getType() const {
    return WEAPON_BOUNCER;
}

void AmmoBouncer::onHitFloorHook() {
    if (speed.y < 0) {
        speed.y = std::min(std::max(-4.0f, speed.y), -1.0f);
    }
}
