#pragma once

#include <memory>

#include "../CarrotQt5.h"
#include "CommonActor.h"

// These need to match the master event table (at CarrotQt5.h) where applicable
// because created objects are directly cast from those values in the event spawner
enum CollectibleType {
    COLLTYPE_FAST_FIRE      = 0x0001,
    COLLTYPE_AMMO_BOUNCER   = 0x0002,
    COLLTYPE_AMMO_SEEKER    = 0x0004,
    COLLTYPE_AMMO_TOASTER   = 0x0006,
    COLLTYPE_GEM_RED        = 0x0040,
    COLLTYPE_GEM_GREEN      = 0x0041,
    COLLTYPE_GEM_BLUE       = 0x0042,
    COLLTYPE_COIN_SILVER    = 0xF002, // temp. assignment
    COLLTYPE_COIN_GOLD      = 0xF003, // temp. assignment
    COLLTYPE_OTHER          = 0xFFFF
};

class Collectible : public CommonActor {

    Q_OBJECT

public:
    Collectible(std::shared_ptr<CarrotQt5> root, enum CollectibleType type, double x = 0.0, double y = 0.0, bool fromEventMap = true);
    ~Collectible();
    void tickEvent();
    enum CollectibleType type;
    void impact(double force_h = 0.0, double force_v = 0.0);
    void DrawUpdate();

private:
    bool intact;
    double phase;
};
