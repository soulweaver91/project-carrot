#include "NormalTurtle.h"

#include "../../CarrotQt5.h"
#include "TurtleShell.h"
#include "../../struct/Constants.h"

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

    if (frozenFramesLeft > 0) {
        return;
    }
    
    if (std::abs(speedX) > EPSILON && !canMoveToPosition(speedX, 0)) {
        setTransition(AnimState::TRANSITION_WITHDRAW, false, [this]() {
            handleTurn(true);
        });
        isTurning = true;
        hurtPlayer = false;
        speedX = 0;
        playSound("ENEMY_TURTLE_WITHDRAW");
    }

    if (!isTurning && !isWithdrawn && !isAttacking) {
        auto players = root->getCollidingPlayer(getHitbox().add(speedX * 64, 0.0));
        if (players.length() > 0) {
            attack();
        }
    }
}

Hitbox EnemyNormalTurtle::getHitbox() {
    return CommonActor::getHitbox(24u, 24u);
}

bool EnemyNormalTurtle::perish() {
    bool goingToPerish = (health == 0);
    if (goingToPerish) {
        root->addActor(std::make_shared<TurtleShell>(root, posX, posY, speedX, -5.0, false));
        Enemy::perish();
    }

    return goingToPerish;
}

void EnemyNormalTurtle::handleTurn(bool isFirstPhase) {
    if (isTurning) {
        if (isFirstPhase) {
            isFacingLeft = !(isFacingLeft);
            setTransition(AnimState::TRANSITION_WITHDRAW_END, false, [this]() {
                handleTurn(false);
            });
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

void EnemyNormalTurtle::attack() {
    setTransition(AnimState::TRANSITION_ATTACK, false, [this]() {
        speedX = (isFacingLeft ? -1 : 1) * 1;
        isAttacking = false;
    });
    speedX = 0;
    isAttacking = true;
    playSound("ENEMY_TURTLE_ATTACK");
    addTimer(4u, false, [this](){
        playSound("ENEMY_TURTLE_ATTACK_2"); 
    });
}
