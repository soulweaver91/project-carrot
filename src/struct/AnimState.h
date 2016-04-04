#pragma once

typedef unsigned AnimStateT;

namespace AnimState {
    // Bits 0, 1: horizontal speed (none, low, med, high)
    unsigned const IDLE   = 0x00000000;
    unsigned const WALK   = 0x00000001;
    unsigned const RUN    = 0x00000002;
    unsigned const DASH   = 0x00000003;

    // Bits 2, 3: vertical speed (none, upwards, downwards, suspended)
    unsigned const V_IDLE = 0x00000000;
    unsigned const JUMP   = 0x00000004;
    unsigned const FALL   = 0x00000008;
    unsigned const HOOK   = 0x0000000c;

    // Bit 4: shoot
    unsigned const SHOOT = 0x00000010;

    // Bits 5-9: multiple special stances that cannot occur together
    // but still have unique bits due to complications in determining
    // the current actor state
    unsigned const CROUCH    = 0x00000020;
    unsigned const LOOKUP    = 0x00000040;
    unsigned const IDLEANIM  = 0x00000080;
    unsigned const BUTTSTOMP = 0x00000100;
    unsigned const UPPERCUT  = 0x00000200;
    unsigned const SIDEKICK  = 0x00000400;
    unsigned const HURT      = 0x00000800;
    unsigned const SWIM      = 0x00001000;
    unsigned const COPTER    = 0x00002000;
    unsigned const PUSH      = 0x00004000;
    unsigned const SWINGROPE = 0x00008000;

    // 30th bit: transition range
    unsigned const TRANSITION_RUN_TO_IDLE                   = 0x40000000;
    unsigned const TRANSITION_RUN_TO_DASH                   = 0x40000001;
    unsigned const TRANSITION_IDLE_FALL_TO_IDLE             = 0x40000002;
    unsigned const TRANSITION_IDLE_TO_IDLE_JUMP             = 0x40000003;
    unsigned const TRANSITION_IDLE_SHOOT_TO_IDLE            = 0x40000004;
    unsigned const TRANSITION_IDLE_FALL_SHOOT_TO_IDLE_SHOOT = 0x40000005;
    unsigned const TRANSITION_IDLE_SHOOT_TO_IDLE_JUMP_SHOOT = 0x40000006;
    unsigned const TRANSITION_UPPERCUT_A                    = 0x40000007;
    unsigned const TRANSITION_UPPERCUT_B                    = 0x40000008;
    unsigned const TRANSITION_END_UPPERCUT                  = 0x40000009;
    unsigned const TRANSITION_BUTTSTOMP_START               = 0x4000000A;
    unsigned const TRANSITION_POLE_H                        = 0x4000000B;
    unsigned const TRANSITION_POLE_V                        = 0x4000000C;
    unsigned const TRANSITION_POLE_H_SLOW                   = 0x4000000D;
    unsigned const TRANSITION_POLE_V_SLOW                   = 0x4000000E;
    unsigned const TRANSITION_DEATH                         = 0x4000000F;
    unsigned const TRANSITION_TURN                          = 0x40000010;
    unsigned const TRANSITION_WITHDRAW                      = 0x40000011;
    unsigned const TRANSITION_WITHDRAW_END                  = 0x40000012;
    unsigned const TRANSITION_WARP                          = 0x40000013;
    unsigned const TRANSITION_WARP_END                      = 0x40000014;

    // 31st bit: UI icon range
    unsigned const UI_PLAYER_FACE       = 0x80000000;
    unsigned const UI_WEAPON_BLASTER    = 0x80000001;
    unsigned const UI_WEAPON_BOUNCER    = 0x80000002;
    unsigned const UI_WEAPON_FREEZER    = 0x80000003;
    unsigned const UI_WEAPON_SEEKER     = 0x80000004;
    unsigned const UI_WEAPON_RF         = 0x80000005;
    unsigned const UI_WEAPON_TOASTER    = 0x80000006;
    unsigned const UI_WEAPON_TNT        = 0x80000007;
    unsigned const UI_WEAPON_PEPPER     = 0x80000008;
    unsigned const UI_WEAPON_ELECTRO    = 0x80000009;
    unsigned const UI_OSD_GEM_RED       = 0x8000000A;
    unsigned const UI_OSD_GEM_GREEN     = 0x8000000B;
    unsigned const UI_OSD_GEM_BLUE      = 0x8000000C;
    unsigned const UI_OSD_GEM_PURPLE    = 0x8000000D;
    unsigned const UI_OSD_COIN_GOLD     = 0x8000000E;
    unsigned const UI_OSD_COIN_SILVER   = 0x8000000F;
    unsigned const UI_OSD_STOPWATCH     = 0x80000010;

    // Aliases for object states overlapping player states
    unsigned const ACTIVATED            = 0x00000020;
    unsigned const TRANSITION_ACTIVATE  = 0x4F000000;

    unsigned const STATE_UNINITIALIZED  = 0xFFFFFFFF;
}
