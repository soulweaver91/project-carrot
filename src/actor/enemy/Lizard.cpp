#include "Lizard.h"

Enemy_Lizard::Enemy_Lizard(std::shared_ptr<CarrotQt5> root, double x, double y) : Enemy(root, x, y) {
    loadResources("Enemy/Lizard");
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
        playSound("ENEMY_LIZARD_SPONTANEOUS");
    }
}

Hitbox Enemy_Lizard::getHitbox() {
    return CommonActor::getHitbox(30u, 30u);
}
