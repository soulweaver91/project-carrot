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
        setTransition(AnimState::TRANSITION_WITHDRAW, false, static_cast<AnimationCallbackFunc>(&EnemyNormalTurtle::handleTurn));
        isTurning = true;
        hurtPlayer = false;
        speedX = 0;
        playSound("ENEMY_TURTLE_WITHDRAW");
    }
}

Hitbox EnemyNormalTurtle::getHitbox() {
    return CommonActor::getHitbox(30u, 30u);
}

void EnemyNormalTurtle::handleTurn(std::shared_ptr<AnimationInstance> animation) {
    if (isTurning) {
        if (!isWithdrawn) {
            isFacingLeft = !(isFacingLeft);
            setTransition(AnimState::TRANSITION_WITHDRAW_END, false, static_cast<AnimationCallbackFunc>(&EnemyNormalTurtle::handleTurn));
            playSound("ENEMY_TURTLE_WITHDRAW_END");
            isWithdrawn = true;
        } else {
            hurtPlayer = true;
            isWithdrawn = false;
            isTurning = false;
            speedX = (isFacingLeft ? -1 : 1) * 1;
        }
    }
}
