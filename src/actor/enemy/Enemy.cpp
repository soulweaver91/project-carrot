#include "Enemy.h"
#include "../../CarrotQt5.h"
#include "../../gamestate/EventMap.h"

Enemy::Enemy(std::shared_ptr<CarrotQt5> root, double x, double y) : CommonActor(root, x, y), hurtPlayer(true) {

}

Enemy::~Enemy() {

}

void Enemy::tickEvent() {
    CommonActor::tickEvent();
}

bool Enemy::canMoveToPosition(double x, double y) {
    Hitbox hitbox = getHitbox();
    short sign = (isFacingLeft ? -1 : 1);

    auto events = root->getGameEvents().lock();

    return ((root->isPositionEmpty(root->calcHitbox(hitbox, x, y - 10), false, shared_from_this())
          || root->isPositionEmpty(root->calcHitbox(hitbox, x, y + 2), false, shared_from_this()))
         && (events != nullptr && (!(events->getPositionEvent(posX + x, posY + y) == PC_AREA_STOP_ENEMY)))
         && (!root->isPositionEmpty(root->calcHitbox(hitbox, x + sign * (hitbox.right - hitbox.left) / 2, y + 32), false, shared_from_this())));
}

bool Enemy::hurtsPlayer() {
    return hurtPlayer;
}
