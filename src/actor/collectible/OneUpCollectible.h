#pragma once
#include "Collectible.h"

class ActorAPI;
class Player;

class OneUpCollectible : public Collectible {
public:
    OneUpCollectible(const ActorInstantiationDetails& initData);
    ~OneUpCollectible();
    virtual void collect(std::shared_ptr<Player> player) override;
};
