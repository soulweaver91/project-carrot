#include "Enemy.h"
#include <algorithm>
#include <cmath>
#include "../../gamestate/ActorAPI.h"
#include "../../gamestate/EventMap.h"
#include "../collectible/CarrotCollectible.h"
#include "../collectible/GemCollectible.h"
#include "../collectible/FastFireCollectible.h"
#include "../weapon/Ammo.h"

Enemy::Enemy(const ActorInstantiationDetails& initData)
    : CommonActor(initData), hurtPlayer(true), isAttacking(false), lastHitDir(NONE) {

}

Enemy::~Enemy() {

}

void Enemy::tickEvent() {
    CommonActor::tickEvent();
}

bool Enemy::perish() {
    if (health == 0) {
        tryGenerateRandomDrop();
        return CommonActor::perish();
    }
    return false;
}

bool Enemy::canMoveToPosition(const CoordinatePair& newPos) {
    short sign = (isFacingLeft ? -1 : 1);

    auto events = api->getGameEvents().lock();

    return ((api->isPositionEmpty(currentHitbox + newPos + CoordinatePair(0, -10), false, shared_from_this())
          || api->isPositionEmpty(currentHitbox + newPos + CoordinatePair(0, 2), false, shared_from_this()))
         && (events != nullptr && (!(events->getPositionEvent(pos + newPos) == PC_AREA_STOP_ENEMY)))
         && (!api->isPositionEmpty(currentHitbox + newPos + CoordinatePair(sign * (currentHitbox.right - currentHitbox.left) / 2, TILE_HEIGHT * 1.0), false, shared_from_this())));
}

void Enemy::tryGenerateRandomDrop(const QVector<QPair<PCEvent, uint>>& dropTable) {
    uint combinedChance = std::accumulate(dropTable.begin(), dropTable.end(), 0, [](const uint& sum, QPair<PCEvent, int> pair) {
        return sum + pair.second;
    });

    uint point = qrand() % combinedChance;
    for (const auto& pair : dropTable) {
        if (point < pair.second) {
            switch (pair.first) {
                case PC_CARROT:
                case PC_FAST_FIRE:
                case PC_GEM_RED:
                case PC_GEM_GREEN:
                case PC_GEM_BLUE:
                    api->createActor(pair.first, pos, {});
                    break;
                default:
                    break;
            }
            return;
        }

        point -= pair.second;
    }
}

bool Enemy::hurtsPlayer() {
    return hurtPlayer && (frozenFramesLeft == 0);
}

void Enemy::handleCollision(std::shared_ptr<CommonActor> other) {
    CommonActor::handleCollision(other);

    // TODO: Use actor type specifying function instead when available
    std::shared_ptr<Ammo> ammo = std::dynamic_pointer_cast<Ammo>(other);
    if (ammo != nullptr) {
        decreaseHealth(ammo->getStrength());
        auto ammoSpeed = ammo->getSpeed();
        if (std::abs(ammoSpeed.x) > EPSILON) {
            lastHitDir = (ammoSpeed.x > 0 ? RIGHT : LEFT);
        } else {
            lastHitDir = (ammoSpeed.y > 0 ? DOWN : UP);
        }
    }
}

const QVector<QPair<PCEvent, uint>> Enemy::defaultDropTable = {
    qMakePair(PC_EMPTY,    10u),
    qMakePair(PC_CARROT,    2u),
    qMakePair(PC_FAST_FIRE, 2u),
    qMakePair(PC_GEM_RED,   3u),
    qMakePair(PC_GEM_GREEN, 2u),
    qMakePair(PC_GEM_BLUE,  1u)
};