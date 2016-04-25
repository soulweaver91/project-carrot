#pragma once
#include "Collectible.h"

class CarrotQt5;
class Player;

class FastFireCollectible : public Collectible {
public:
    FastFireCollectible(std::shared_ptr<CarrotQt5> root, double x, double y, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;
};
