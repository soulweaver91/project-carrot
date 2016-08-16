#pragma once
#include "Collectible.h"

class ActorAPI;
class Player;

class CarrotCollectible : public Collectible {
public:
    CarrotCollectible(std::shared_ptr<ActorAPI> api, double x, double y, bool maxCarrot, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;

private:
    bool maxCarrot;
};
