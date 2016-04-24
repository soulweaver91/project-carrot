#pragma once

#include <memory>

#include "CommonActor.h"

class CarrotQt5;
class GameView;

// These need to match the master event table (at CarrotQt5.h) where applicable
// because created objects are directly cast from those values in the event spawner
enum CollectibleType {
    COLLTYPE_FAST_FIRE      = 0x0001,
    COLLTYPE_AMMO_BOUNCER   = 0x0002,
    COLLTYPE_AMMO_FREEZER   = 0x0003,
    COLLTYPE_AMMO_SEEKER    = 0x0004,
    COLLTYPE_AMMO_RF        = 0x0005,
    COLLTYPE_AMMO_TOASTER   = 0x0006,
    COLLTYPE_AMMO_TNT       = 0x0007,
    COLLTYPE_AMMO_PEPPER    = 0x0008,
    COLLTYPE_AMMO_ELECTRO   = 0x0009,
    COLLTYPE_GEM_RED        = 0x0040,
    COLLTYPE_GEM_GREEN      = 0x0041,
    COLLTYPE_GEM_BLUE       = 0x0042,
    COLLTYPE_COIN_SILVER    = 0x0048,
    COLLTYPE_COIN_GOLD      = 0x0049,
    COLLTYPE_OTHER          = 0xFFFF
};

class Collectible : public CommonActor {
public:
    Collectible(std::shared_ptr<CarrotQt5> root, enum CollectibleType type, double x = 0.0, double y = 0.0, bool fromEventMap = true);
    ~Collectible();
    void tickEvent();
    enum CollectibleType type;
    void impact(double forceX = 0.0, double forceY = 0.0);
    void drawUpdate(std::shared_ptr<GameView>& view) override;

private:
    bool untouched;
    double phase;
    void setFacingDirection();
};
