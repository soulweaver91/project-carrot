#include "NormalTurtle.h"

#include <cmath>
#include "TurtleShell.h"
#include "../../gamestate/ActorAPI.h"
#include "../../struct/Constants.h"

EnemyNormalTurtle::EnemyNormalTurtle(const ActorInstantiationDetails& initData)
    : Enemy(initData), isTurning(false), isWithdrawn(false) {
    loadResources("Enemy/Turtle");
    setAnimation(AnimState::WALK);
    speed.x = 1;
}

EnemyNormalTurtle::~EnemyNormalTurtle() {

}

void EnemyNormalTurtle::tickEvent() {
    Enemy::tickEvent();

    if (frozenFramesLeft > 0) {
        return;
    }
    
    if (std::abs(speed.x) > EPSILON && !canMoveToPosition({ speed.x, 0.0f })) {
        setTransition(AnimState::TRANSITION_WITHDRAW, false, [this]() {
            handleTurn(true);
        });
        isTurning = true;
        hurtPlayer = false;
        speed.x = 0;
        playSound("ENEMY_TURTLE_WITHDRAW");
    }

    if (!isTurning && !isWithdrawn && !isAttacking) {
        auto players = api->getCollidingPlayer(currentHitbox + sf::Vector2f(speed.x * 64, 0.0f));
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
        api->addActor(std::make_shared<TurtleShell>(ActorInstantiationDetails(api, pos, false), sf::Vector2f(speed.x, -5.0f)));
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
            speed.x = (isFacingLeft ? -1 : 1) * 1;
        }
    }
}

void EnemyNormalTurtle::attack() {
    setTransition(AnimState::TRANSITION_ATTACK, false, [this]() {
        speed.x = (isFacingLeft ? -1 : 1) * 1;
        isAttacking = false;
    });
    speed.x = 0;
    isAttacking = true;
    playSound("ENEMY_TURTLE_ATTACK");
    addTimer(4u, false, [this](){
        playSound("ENEMY_TURTLE_ATTACK_2"); 
    });
}
