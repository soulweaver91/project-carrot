#pragma once
#ifdef CARROT_DEBUG

#define DEBUG_VARS_SIZE 32

#include <QString>

struct DebugConfig {
    bool dbgOverlaysActive;
    bool dbgShowMasked;
    int tempModifier[DEBUG_VARS_SIZE]; // temporary variables for testing new features
    QString tempModifierName[DEBUG_VARS_SIZE];
    unsigned char currentTempModifier;
};

#endif
