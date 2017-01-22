#include "FastFireCollectible.h"
#include "../Player.h"

FastFireCollectible::FastFireCollectible(const ActorInstantiationDetails& initData)
    : Collectible(initData) {
    scoreValue = 200;
    setFacingDirection();

    AnimationUser::setAnimation("PICKUP_FASTFIRE");
    setAnimation(AnimState::IDLE);
}

void FastFireCollectible::collect(std::shared_ptr<Player> player) {
    player->addFastFire(1);
    Collectible::collect(player);
}
