#pragma once
#include "Collectible.h"
#include "../../struct/WeaponTypes.h"

class ActorAPI;
class Player;

class AmmoCollectible : public Collectible {
public:
    AmmoCollectible(std::shared_ptr<ActorAPI> api, double x, double y, WeaponType type, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;

private:
    const WeaponType weaponType;
};
