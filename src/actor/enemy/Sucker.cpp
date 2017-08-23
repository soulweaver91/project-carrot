#include "Sucker.h"

#include <cmath>
#include "../../gamestate/ActorAPI.h"
#include "../../struct/Constants.h"

EnemySucker::EnemySucker(const ActorInstantiationDetails& initData, LastHitDirection dir)
    : Enemy(initData), cycle(0) {
    loadResources("Enemy/Sucker");
    maxHealth = 4;
    setAnimation(AnimState::WALK);

    if (dir != NONE) {
        isFacingLeft = dir == LEFT;
        health = 1;
        isGravityAffected = false;
        setTransition(1073741824, false, [this]() {
            speed.x = 0;
            setAnimation(AnimState::WALK);
            isGravityAffected = true;
        });
        if (dir == LEFT || dir == RIGHT) {
            speed.x = 3 * (dir == LEFT ? -1 : 1);
        }
        playSound("ENEMY_SUCKER_DEFLATE");
    } else {
        health = 4;
    }
}

EnemySucker::~EnemySucker() {
}

void EnemySucker::tickEvent() {
    if (!inTransition && std::abs(speed.y) < EPSILON && std::abs(speed.x) > EPSILON && !canMoveToPosition({ speed.x, 0.0f })) {
        isFacingLeft = !isFacingLeft;
        speed.x *= -1;
    }

    if (!inTransition && frozenFramesLeft == 0) {
        cycle = (cycle + 1) % (7 * 12);
        if (cycle == 0) {
            playSound("ENEMY_SUCKER_WALK_1");
        }
        if (cycle == (7 * 7)) {
            playSound("ENEMY_SUCKER_WALK_2");
        }
        if (cycle == (3 * 7) || cycle == (8 * 7)) {
            playSound("ENEMY_SUCKER_WALK_3");
        }
        if ((cycle >= (6 * 7) && cycle < (8 * 7)) || cycle >= (10 * 7)) {
            speed.x = 0.5 * (isFacingLeft ? -1 : 1);
        } else {
            speed.x = 0;
        }
    }

    CommonActor::tickEvent();
}
