#include "Lizard.h"

EnemyLizard::EnemyLizard(std::shared_ptr<ActorAPI> api, double x, double y) : Enemy(api, x, y - 6.0) {
    loadResources("Enemy/Lizard");
    setAnimation(AnimState::WALK);
    speedX = 1.0;
}

EnemyLizard::~EnemyLizard() {

}

void EnemyLizard::tickEvent() {
    Enemy::tickEvent();
    
    if (!canMoveToPosition(speedX, 0.0)) {
        isFacingLeft = !(isFacingLeft);
        speedX = (isFacingLeft ? -1.0 : 1.0) * 1.0;
    }

    if (qrand() % 10000 < 4) {
        playSound("ENEMY_LIZARD_SPONTANEOUS");
    }
}

void EnemyLizard::updateHitbox() {
    CommonActor::updateHitbox(30u, 30u);
}
