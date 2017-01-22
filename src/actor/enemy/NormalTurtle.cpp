#include "NormalTurtle.h"

#include <cmath>
#include "TurtleShell.h"
#include "../../gamestate/ActorAPI.h"
#include "../../struct/Constants.h"

EnemyNormalTurtle::EnemyNormalTurtle(const ActorInstantiationDetails& initData)
    : Enemy(initData), isTurning(false), isWithdrawn(false) {
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
        auto players = api->getCollidingPlayer(currentHitbox + CoordinatePair(speedX * 64, 0.0));
        if (players.length() > 0) {
            attack();
        }
    }
}

void EnemyNormalTurtle::updateHitbox() {
    CommonActor::updateHitbox(24u, 24u);
}

bool EnemyNormalTurtle::perish() {
    bool goingToPerish = (health == 0);
    if (goingToPerish) {
        api->addActor(std::make_shared<TurtleShell>(ActorInstantiationDetails(api, { posX, posY }, false), speedX, -5.0));
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
