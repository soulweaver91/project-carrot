#pragma once
#include "Collectible.h"

class CarrotQt5;
class Player;

enum CoinType {
    COIN_SILVER = 0,
    COIN_GOLD = 1
};

class CoinCollectible : public Collectible {
public:
    CoinCollectible(std::shared_ptr<CarrotQt5> root, double x, double y, CoinType type, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;

private:
    const CoinType coinType;
    static const uint coinValues[2];
};
