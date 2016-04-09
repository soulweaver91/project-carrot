#include "Lizard.h"

EnemyLizard::EnemyLizard(std::shared_ptr<CarrotQt5> root, double x, double y) : Enemy(root, x, y) {
    loadResources("Enemy/Lizard");
    setAnimation(AnimState::WALK);
    speedX = 1;
}

EnemyLizard::~EnemyLizard() {

}

void EnemyLizard::tickEvent() {
    Enemy::tickEvent();
    
    if (!canMoveToPosition(speedX,0)) {
        isFacingLeft = !(isFacingLeft);
        speedX = (isFacingLeft ? -1 : 1) * 1;
    }

    if (qrand() % 10000 < 4) {
        playSound("ENEMY_LIZARD_SPONTANEOUS");
    }
}

Hitbox EnemyLizard::getHitbox() {
    return CommonActor::getHitbox(30u, 30u);
}
