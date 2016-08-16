#include "CoinCollectible.h"
#include "../Player.h"

const uint CoinCollectible::coinValues[2] = {
    500,
    1000
};

CoinCollectible::CoinCollectible(std::shared_ptr<ActorAPI> api, double x, double y, CoinType type, bool fromEventMap)
    : Collectible(api, x, y, fromEventMap), coinType(type) {
    scoreValue = coinValues[(uint)type % 2];
    switch (coinType) {
        case COIN_GOLD:    AnimationUser::setAnimation("PICKUP_COIN_GOLD");   break;
        case COIN_SILVER:  AnimationUser::setAnimation("PICKUP_COIN_SILVER"); break;
    }
    setFacingDirection();
}

void CoinCollectible::collect(std::shared_ptr<Player> player) {
    player->addCoins(coinType, 1);
    Collectible::collect(player);
}
