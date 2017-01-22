#pragma once
#include "Collectible.h"
#include "../../struct/PCEvent.h"

class ActorAPI;
class Player;

class FoodCollectible : public Collectible {
public:
    FoodCollectible(const ActorInstantiationDetails& initData, PCEvent type);
    virtual void collect(std::shared_ptr<Player> player) override;

private:
    bool isDrinkable;
};
