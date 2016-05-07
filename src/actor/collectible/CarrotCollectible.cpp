#include "CarrotCollectible.h"
#include "../Player.h"

CarrotCollectible::CarrotCollectible(std::shared_ptr<CarrotQt5> root, double x, double y, bool maxCarrot, bool fromEventMap)
    : Collectible(root, x, y, fromEventMap), maxCarrot(maxCarrot) {
    setFacingDirection();

    if (maxCarrot) {
        scoreValue = 100;
        AnimationUser::setAnimation("PICKUP_CARROT_FULL");
    } else {
        scoreValue = 100;
        AnimationUser::setAnimation("PICKUP_CARROT");
    }

    setAnimation(AnimState::IDLE);
}

void CarrotCollectible::collect(std::shared_ptr<Player> player) {
    player->addHealth(maxCarrot ? -1 : 1);
    Collectible::collect(player);
}
