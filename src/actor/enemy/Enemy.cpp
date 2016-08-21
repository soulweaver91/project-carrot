#include "Enemy.h"
#include <algorithm>
#include "../../gamestate/ActorAPI.h"
#include "../../gamestate/EventMap.h"
#include "../collectible/CarrotCollectible.h"
#include "../collectible/GemCollectible.h"
#include "../collectible/FastFireCollectible.h"
#include "../weapon/Ammo.h"

Enemy::Enemy(std::shared_ptr<ActorAPI> api, double x, double y)
    : CommonActor(api, x, y), hurtPlayer(true), isAttacking(false), lastHitDir(NONE) {

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

bool Enemy::canMoveToPosition(double x, double y) {
    Hitbox hitbox = getHitbox();
    short sign = (isFacingLeft ? -1 : 1);

    auto events = api->getGameEvents().lock();

    return ((api->isPositionEmpty(hitbox + CoordinatePair(x, y - 10), false, shared_from_this())
          || api->isPositionEmpty(hitbox + CoordinatePair(x, y + 2), false, shared_from_this()))
         && (events != nullptr && (!(events->getPositionEvent(posX + x, posY + y) == PC_AREA_STOP_ENEMY)))
         && (!api->isPositionEmpty(hitbox + CoordinatePair(x + sign * (hitbox.right - hitbox.left) / 2, y + 32), false, shared_from_this())));
}

void Enemy::tryGenerateRandomDrop(const QVector<QPair<PCEvent, uint>>& dropTable) {
    uint combinedChance = std::accumulate(dropTable.begin(), dropTable.end(), 0, [](const uint& sum, QPair<PCEvent, int> pair) {
        return sum + pair.second;
    });

    uint point = qrand() % combinedChance;
    for (const auto& pair : dropTable) {
        if (point < pair.second) {
            int i = 0;
            switch (pair.first) {
                case PC_CARROT:
                    api->addActor(std::make_shared<CarrotCollectible>(api, posX, posY, false, false));
                    break;
                case PC_FAST_FIRE:
                    api->addActor(std::make_shared<FastFireCollectible>(api, posX, posY, false));
                    break;
                case PC_GEM_BLUE:
                    i++;
                case PC_GEM_GREEN:
                    i++;
                case PC_GEM_RED:
                    api->addActor(std::make_shared<GemCollectible>(api, posX, posY, (GemType)((int)GEM_RED + i), false));
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
        double ammoSpeedX = ammo->getSpeedX();
        double ammoSpeedY = ammo->getSpeedY();
        if (std::abs(ammoSpeedX) > EPSILON) {
            lastHitDir = (ammoSpeedX > 0 ? RIGHT : LEFT);
        } else {
            lastHitDir = (ammoSpeedY > 0 ? DOWN : UP);
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