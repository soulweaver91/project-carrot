#include "Lizard.h"

Enemy_Lizard::Enemy_Lizard(std::shared_ptr<CarrotQt5> root, double x, double y) : Enemy(root, x, y) {
    unsigned a = addAnimation(AnimState::WALK,"lizard/walk.png", 12,1,72,56,10,36,53);
    assignAnimation(AnimState::IDLE, a);
    assignAnimation(AnimState::RUN, a);
    assignAnimation(AnimState::FALL, a);
    assignAnimation(AnimState::WALK | AnimState::FALL, a);

    setAnimation(AnimState::WALK);
    speed_h = 1;
}

Enemy_Lizard::~Enemy_Lizard() {

}

void Enemy_Lizard::tickEvent() {
    Enemy::tickEvent();
    
    if (!canMoveToPosition(speed_h,0)) {
        facingLeft = !(facingLeft);
        speed_h = (facingLeft ? -1 : 1) * 1;
    }

    if (qrand() % 10000 < 4) {
        playSound(SFX_LIZARD_SPONTANEOUS);
    }
}

Hitbox Enemy_Lizard::getHitbox() {
    Hitbox box = {pos_x - 20,
                  pos_y - 30,
                  pos_x + 20,
                  pos_y
    };
    return box;
}
