#include "NormalTurtle.h"
#include "../Enemy.h"
#include "../CommonActor.h"
#include "../CarrotQt5.h"

Enemy_NormalTurtle::Enemy_NormalTurtle(CarrotQt5* root, double x, double y) : Enemy(root, x, y), isTurning(false), isWithdrawn(false) {
    unsigned a = addAnimation(AnimState::WALK,"Data/Assets/nt_walk.png",4,3,70,57,10,35,52);
    assignAnimation(AnimState::IDLE, a);
    assignAnimation(AnimState::RUN, a);
    assignAnimation(AnimState::FALL, a);
    assignAnimation(AnimState::WALK | AnimState::FALL, a);
    
    addAnimation(AnimState::TRANSITION_WITHDRAW,"Data/Assets/nt_shell_in.png",1,7,73,44,10,26,38);
    addAnimation(AnimState::TRANSITION_WITHDRAW_END,"Data/Assets/nt_shell_out.png",1,6,73,44,10,26,38);

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
    Hitbox box = {pos_x - 20,
                  pos_y - 30,
                  pos_x + 20,
                  pos_y
    };
    return box;
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
