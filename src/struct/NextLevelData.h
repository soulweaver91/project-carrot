#pragma once

#include "Constants.h"
#include "WeaponTypes.h"
#include "GameDifficulty.h"

#include <QtGlobal>

enum ExitType {
    NEXT_NONE,
    NEXT_NORMAL,
    NEXT_WARP,
    NEXT_BONUS,
    NEXT_SPECIAL
};

struct PlayerCarryOver {
    uint lives;
    uint ammo[WEAPONCOUNT];
    bool poweredUp[WEAPONCOUNT];
    uint fastfires;
    uint score;
    uint foodCounter;
    WeaponType currentWeapon;
};

struct NextLevelData {
    QString levelName;
    QString episodeName;
    GameDifficulty difficulty;
    ExitType exitType;
    PlayerCarryOver playerCarryOvers[32];

    NextLevelData(QString levelName, QString episodeName = "", GameDifficulty difficulty = DIFFICULTY_NORMAL, ExitType exitType = NEXT_NONE)
    : levelName(levelName), episodeName(episodeName), difficulty(difficulty), exitType(exitType) {

    }
};
