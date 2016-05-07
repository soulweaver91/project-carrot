#pragma once
#include "Collectible.h"

class CarrotQt5;
class Player;

class CarrotCollectible : public Collectible {
public:
    CarrotCollectible(std::shared_ptr<CarrotQt5> root, double x, double y, bool maxCarrot, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;

private:
    bool maxCarrot;
};
