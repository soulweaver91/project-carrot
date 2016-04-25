#pragma once
#include "Collectible.h"
#include "../../struct/WeaponTypes.h"

class CarrotQt5;
class Player;

class AmmoCollectible : public Collectible {
public:
    AmmoCollectible(std::shared_ptr<CarrotQt5> root, WeaponType type, double x, double y, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;

private:
    const WeaponType weaponType;
};
