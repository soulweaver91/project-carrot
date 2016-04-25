#include "FastFireCollectible.h"
#include "../Player.h"

FastFireCollectible::FastFireCollectible(std::shared_ptr<CarrotQt5> root, double x, double y, bool fromEventMap)
    : Collectible(root, x, y, fromEventMap) {
    scoreValue = 100;
    setFacingDirection();

    AnimationUser::setAnimation("PICKUP_FASTFIRE");
    setAnimation(AnimState::IDLE);
}

void FastFireCollectible::collect(std::shared_ptr<Player> player) {
    player->addFastFire(1);
    Collectible::collect(player);
}