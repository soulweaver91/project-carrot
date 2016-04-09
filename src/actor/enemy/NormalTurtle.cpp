#include "NormalTurtle.h"

EnemyNormalTurtle::EnemyNormalTurtle(std::shared_ptr<CarrotQt5> root, double x, double y)
    : Enemy(root, x, y), isTurning(false), isWithdrawn(false) {
    loadResources("Enemy/Turtle");
    setAnimation(AnimState::WALK);
    speedX = 1;
}

EnemyNormalTurtle::~EnemyNormalTurtle() {

}

void EnemyNormalTurtle::tickEvent() {
    Enemy::tickEvent();
    
    if (!canMoveToPosition(speedX, 0)) {
        setTransition(AnimState::TRANSITION_WITHDRAW, false);
        isTurning = true;
        hurtPlayer = false;
        speedX = 0;
    }
}

Hitbox EnemyNormalTurtle::getHitbox() {
    return CommonActor::getHitbox(30u, 30u);
}

void EnemyNormalTurtle::onTransitionEndHook() {
    if (isTurning) {
        if (!isWithdrawn) {
            isFacingLeft = !(isFacingLeft);
            setTransition(AnimState::TRANSITION_WITHDRAW_END, false);
            isWithdrawn = true;
        } else {
            hurtPlayer = true;
            isWithdrawn = false;
            isTurning = false;
            speedX = (isFacingLeft ? -1 : 1) * 1;
        }
    }
}
