#pragma once
#include "Collectible.h"
#include "../../struct/PCEvent.h"

class CarrotQt5;
class Player;

class FoodCollectible : public Collectible {
public:
    FoodCollectible(std::shared_ptr<CarrotQt5> root, double x, double y, PCEvent type, bool fromEventMap = true);
    virtual void collect(std::shared_ptr<Player> player) override;

private:
    bool isDrinkable;
};
