#include "Lizard.h"

EnemyLizard::EnemyLizard(const ActorInstantiationDetails& initData) : Enemy(initData) {
    loadResources("Enemy/Lizard");
    setAnimation(AnimState::WALK);
    pos.y -= 6.0;
    speed.x = 1.0;
}

EnemyLizard::~EnemyLizard() {

}

void EnemyLizard::tickEvent() {
    Enemy::tickEvent();
    
    if (!canMoveToPosition({ speed.x, 0.0f })) {
        isFacingLeft = !(isFacingLeft);
        speed.x = (isFacingLeft ? -1.0 : 1.0) * 1.0;
    }

    if (qrand() % 10000 < 4) {
        playSound("ENEMY_LIZARD_SPONTANEOUS");
    }
}

void EnemyLizard::updateHitbox() {
    CommonActor::updateHitbox(30u, 30u);
}
