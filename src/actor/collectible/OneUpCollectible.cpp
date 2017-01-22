#include "OneUpCollectible.h"
#include "../Player.h"

OneUpCollectible::OneUpCollectible(const ActorInstantiationDetails& initData)
    : Collectible(initData) {
    scoreValue = 1000;

    AnimationUser::setAnimation("PICKUP_ONEUP");
    setAnimation(AnimState::IDLE);
}

OneUpCollectible::~OneUpCollectible() {

}

void OneUpCollectible::collect(std::shared_ptr<Player> player) {
    player->addLives(1);
    Collectible::collect(player);
}
