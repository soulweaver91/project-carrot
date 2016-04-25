#pragma once

#include <memory>

#include "../CommonActor.h"

class CarrotQt5;
class GameView;
class Player;

// These need to match the master event table (at CarrotQt5.h) where applicable
// because created objects are directly cast from those values in the event spawner
class Collectible : public CommonActor {
public:
    Collectible(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0, bool fromEventMap = true);
    ~Collectible();
    void tickEvent();
    void impact(double forceX = 0.0, double forceY = 0.0);
    void drawUpdate(std::shared_ptr<GameView>& view) override;
    virtual void collect(std::shared_ptr<Player> player);

protected:
    bool untouched;
    uint scoreValue;
    double phase;
    void setFacingDirection();
};
