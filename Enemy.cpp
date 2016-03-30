#include <memory>
#include "Enemy.h"
#include "EventMap.h"
#include "CommonActor.h"
#include "CarrotQt5.h"

Enemy::Enemy(std::shared_ptr<CarrotQt5> root, double x, double y) : CommonActor(root, x, y), hurtPlayer(true) {

}

Enemy::~Enemy() {

}

void Enemy::tickEvent() {
    CommonActor::tickEvent();
}

bool Enemy::canMoveToPosition(double x, double y) {
    Hitbox h = getHitbox();
    short sign = (facingLeft ? -1 : 1);

    return ((root->isPositionEmpty(root->calcHitbox(h,x,y - 10),false,this)
          || root->isPositionEmpty(root->calcHitbox(h,x,y + 2),false,this))
         && (!(root->game_events->getPositionEvent(pos_x + x, pos_y + y) == PC_AREA_STOP_ENEMY))
         && (!root->isPositionEmpty(root->calcHitbox(h,x + sign * (h.right - h.left) / 2,y + 32),false,this)));
}

bool Enemy::hurtsPlayer() {
    return hurtPlayer;
}
