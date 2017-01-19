#pragma once

#include "Constants.h"
#include "WeaponTypes.h"

#include <QtGlobal>

enum ExitType {
    NEXT_NONE,
    NEXT_NORMAL,
    NEXT_WARP,
    NEXT_BONUS,
    NEXT_SPECIAL
};

struct LevelCarryOver {
    uint lives;
    uint ammo[WEAPONCOUNT];
    bool poweredUp[WEAPONCOUNT];
    uint fastfires;
    uint score;
    uint foodCounter;
    WeaponType currentWeapon;
    ExitType exitType;
};
