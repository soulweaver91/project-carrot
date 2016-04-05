#include "NormalTurtle.h"

Enemy_NormalTurtle::Enemy_NormalTurtle(std::shared_ptr<CarrotQt5> root, double x, double y)
    : Enemy(root, x, y), isTurning(false), isWithdrawn(false) {
    loadResources("Enemy/Turtle");
    setAnimation(AnimState::WALK);
    speed_h = 1;
}

Enemy_NormalTurtle::~Enemy_NormalTurtle() {

}

void Enemy_NormalTurtle::tickEvent() {
    Enemy::tickEvent();
    
    if (!canMoveToPosition(speed_h,0)) {
        setTransition(AnimState::TRANSITION_WITHDRAW,false);
        isTurning = true;
        hurtPlayer = false;
        speed_h = 0;
    }
}

Hitbox Enemy_NormalTurtle::getHitbox() {
    return CommonActor::getHitbox(30u, 30u);
}

void Enemy_NormalTurtle::onTransitionEndHook() {
    if (isTurning) {
        if (!isWithdrawn) {
            facingLeft = !(facingLeft);
            setTransition(AnimState::TRANSITION_WITHDRAW_END,false);
            isWithdrawn = true;
        } else {
            hurtPlayer = true;
            isWithdrawn = false;
            isTurning = false;
            speed_h = (facingLeft ? -1 : 1) * 1;
        }
    }
}
