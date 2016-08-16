#pragma once
#include "Collectible.h"

class ActorAPI;
class Player;

enum CoinType {
    COIN_SILVER = 0,
    COIN_GOLD = 1
};

class CoinCollectible : public Collectible {
public:
    CoinCollectible(std::shared_ptr<ActorAPI> api, double x, double y, CoinType type, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;

private:
    const CoinType coinType;
    static const uint coinValues[2];
};
