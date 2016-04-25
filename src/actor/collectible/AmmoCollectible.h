#pragma once
#include "Collectible.h"
#include "../../struct/WeaponTypes.h"

class CarrotQt5;
class Player;

class AmmoCollectible : public Collectible {
public:
    AmmoCollectible(std::shared_ptr<CarrotQt5> root, double x, double y, WeaponType type, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;

private:
    const WeaponType weaponType;
};
