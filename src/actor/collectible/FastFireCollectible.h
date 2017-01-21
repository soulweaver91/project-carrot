#pragma once
#include "Collectible.h"

class ActorAPI;
class Player;

class FastFireCollectible : public Collectible {
public:
    FastFireCollectible(const ActorInstantiationDetails& initData, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;
};
