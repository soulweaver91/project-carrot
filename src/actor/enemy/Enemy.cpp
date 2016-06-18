#include "Enemy.h"
#include <algorithm>
#include "../../CarrotQt5.h"
#include "../../gamestate/EventMap.h"
#include "../collectible/CarrotCollectible.h"
#include "../collectible/GemCollectible.h"
#include "../collectible/FastFireCollectible.h"

Enemy::Enemy(std::shared_ptr<CarrotQt5> root, double x, double y) 
    : CommonActor(root, x, y), hurtPlayer(true), isAttacking(false) {

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

    auto events = root->getGameEvents().lock();

    return ((root->isPositionEmpty(hitbox + CoordinatePair(x, y - 10), false, shared_from_this())
          || root->isPositionEmpty(hitbox + CoordinatePair(x, y + 2), false, shared_from_this()))
         && (events != nullptr && (!(events->getPositionEvent(posX + x, posY + y) == PC_AREA_STOP_ENEMY)))
         && (!root->isPositionEmpty(hitbox + CoordinatePair(x + sign * (hitbox.right - hitbox.left) / 2, y + 32), false, shared_from_this())));
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
                    root->addActor(std::make_shared<CarrotCollectible>(root, posX, posY, false, false));
                    break;
                case PC_FAST_FIRE:
                    root->addActor(std::make_shared<FastFireCollectible>(root, posX, posY, false));
                    break;
                case PC_GEM_BLUE:
                    i++;
                case PC_GEM_GREEN:
                    i++;
                case PC_GEM_RED:
                    root->addActor(std::make_shared<GemCollectible>(root, posX, posY, (GemType)((int)GEM_RED + i), false));
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
    return hurtPlayer;
}

const QVector<QPair<PCEvent, uint>> Enemy::defaultDropTable = {
    qMakePair(PC_EMPTY,    10u),
    qMakePair(PC_CARROT,    2u),
    qMakePair(PC_FAST_FIRE, 2u),
    qMakePair(PC_GEM_RED,   3u),
    qMakePair(PC_GEM_GREEN, 2u),
    qMakePair(PC_GEM_BLUE,  1u)
};