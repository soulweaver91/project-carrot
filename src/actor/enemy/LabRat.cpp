#include "LabRat.h"

#include <cmath>
#include "../../gamestate/ActorAPI.h"
#include "../../struct/Constants.h"
#include "../Player.h"

EnemyLabRat::EnemyLabRat(const ActorInstantiationDetails& initData)
    : Enemy(initData), canAttack(true), idling(false), canIdle(false) {
    loadResources("Enemy/LabRat");
    setAnimation(AnimState::WALK);
    speed.x = 1;

    addTimer(420u + qrand() % 140u, false, [this]() {
        canIdle = true;
    });
}

EnemyLabRat::~EnemyLabRat() {

}

void EnemyLabRat::tickEvent() {
    Enemy::tickEvent();

    if (idling || frozenFramesLeft > 0) {
        return;
    }

    if (!isAttacking) {
        if (!canMoveToPosition({ speed.x, 0.0f })) {
            isFacingLeft = !(isFacingLeft);
            speed.x = (isFacingLeft ? -1 : 1) * 1;
        }

        if (canAttack && std::abs(speed.y) < EPSILON) {
            auto players = api->getCollidingPlayer(Hitbox(currentHitbox).extend(
                isFacingLeft ? 128.0 : 0.0, 20.0,
                isFacingLeft ? 0.0 : 128.0, 20.0
            ));
            if (players.length() > 0) {
                attack();
            }
        }

        if (qrand() % 10000 < 4) {
            playSound("ENEMY_LAB_RAT_SPONTANEOUS");
        }

        if (canIdle && qrand() % 50 == 0) {
            speed.x = 0;
            idling = true;
            AnimationUser::setAnimation("ENEMY_LAB_RAT_IDLE");
            canIdle = false;

            addTimer(120u, false, [this]() {
                playSound("ENEMY_LAB_RAT_IDLE");
            });

            addTimer(240u, false, [this]() {
                idling = false;
                setAnimation(AnimState::WALK);
                speed.x = (isFacingLeft ? -1 : 1) * 1;

                addTimer(420u + qrand() % 140u, false, [this]() {
                    canIdle = true;
                });
            });
        }
    } else {
        internalForce.y += 0.08;
    }
}

void EnemyLabRat::attack() {
    setTransition(AnimState::TRANSITION_ATTACK, false, [this]() {
        speed.x = (isFacingLeft ? -1 : 1) * 1;
        isAttacking = false;
        canAttack = false;

        addTimer(70u, false, [this]() {
            canAttack = true;
        });
    });
    speed.x = (isFacingLeft ? -1 : 1) * 2;
    moveInstantly({ 0, -1 }, false);
    speed.y = -1;
    internalForce.y = 0.5;
    isAttacking = true;
    canJump = false;
    playSound("ENEMY_LAB_RAT_ATTACK");
}
