#pragma once

// These need to match the other event tables (at the moment only at Collectible.h) where applicable
// because created objects are directly cast from these values in the event spawner
enum PCEvent {
    PC_EMPTY                = 0x0000,
    PC_FAST_FIRE            = 0x0001,
    PC_AMMO_BOUNCER         = 0x0002,
    PC_AMMO_FREEZER         = 0x0003,
    PC_AMMO_SEEKER          = 0x0004,
    PC_AMMO_RF              = 0x0005,
    PC_AMMO_TOASTER         = 0x0006,
    PC_AMMO_TNT             = 0x0007,
    PC_AMMO_PEPPER          = 0x0008,
    PC_AMMO_ELECTRO         = 0x0009,
    PC_POWERUP_BLASTER      = 0x0011, // not implemented
    PC_POWERUP_BOUNCER      = 0x0012, // not implemented
    PC_POWERUP_FREEZER      = 0x0013, // not implemented
    PC_POWERUP_SEEKER       = 0x0014, // not implemented
    PC_POWERUP_RF           = 0x0015, // not implemented
    PC_POWERUP_TOASTER      = 0x0016, // not implemented
    PC_POWERUP_TNT          = 0x0017, // not implemented
    PC_POWERUP_PEPPER       = 0x0018, // not implemented
    PC_POWERUP_ELECTRO      = 0x0019, // not implemented
    PC_GEM_RED              = 0x0040,
    PC_GEM_GREEN            = 0x0041,
    PC_GEM_BLUE             = 0x0042,
    PC_SAVE_POINT           = 0x005A,
    PC_SPRING_RED           = 0x00C0,
    PC_SPRING_GREEN         = 0x00C1,
    PC_SPRING_BLUE          = 0x00C2,
    PC_JAZZ_LEVEL_START     = 0x0100,
    PC_SPAZ_LEVEL_START     = 0x0101, // not implemented
    PC_LORI_LEVEL_START     = 0x0102, // not implemented
    PC_AREA_EOL             = 0x0108,
    PC_WARP_ORIGIN          = 0x010E,
    PC_WARP_TARGET          = 0x010F,
    PC_MODIFIER_VINE        = 0x0110,
    PC_MODIFIER_ONE_WAY     = 0x0111,
    PC_MODIFIER_HOOK        = 0x0112, // not implemented
    PC_MODIFIER_H_POLE      = 0x0113,
    PC_MODIFIER_V_POLE      = 0x0114,
    PC_SCENERY_DESTRUCT     = 0x0115,
    PC_SCENERY_BUTTSTOMP    = 0x0117,
    PC_MODIFIER_HURT        = 0x0118,
    PC_LIGHT_SET            = 0x0120,
    PC_LIGHT_RESET          = 0x0125,
    PC_ENEMY_TURTLE_NORMAL  = 0x0180,
    PC_ENEMY_LIZARD         = 0x0184,
    PC_PUSHABLE_ROCK        = 0x005C,
    PC_TRIGGER_CRATE        = 0x005B,
    PC_TRIGGER_AREA         = 0x0119,
    PC_MODIFIER_RICOCHET    = 0x011A,
    PC_BRIDGE               = 0x00C3,
    PC_AREA_STOP_ENEMY      = 0x0143,
    PC_COIN_SILVER          = 0x0048,
    PC_COIN_GOLD            = 0x0049,
    PC_MOVING_PLATFORM      = 0x00C4
};