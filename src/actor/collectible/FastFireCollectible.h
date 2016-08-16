#pragma once
#include "Collectible.h"

class ActorAPI;
class Player;

class FastFireCollectible : public Collectible {
public:
    FastFireCollectible(std::shared_ptr<ActorAPI> api, double x, double y, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;
};
