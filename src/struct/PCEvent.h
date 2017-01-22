#pragma once

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
    PC_POWERUP_BLASTER      = 0x0011,
    PC_POWERUP_BOUNCER      = 0x0012,
    PC_POWERUP_FREEZER      = 0x0013,
    PC_POWERUP_SEEKER       = 0x0014,
    PC_POWERUP_RF           = 0x0015,
    PC_POWERUP_TOASTER      = 0x0016,
    PC_POWERUP_TNT          = 0x0017,
    PC_POWERUP_PEPPER       = 0x0018,
    PC_POWERUP_ELECTRO      = 0x0019,
    PC_GEM_RED              = 0x0040,
    PC_GEM_GREEN            = 0x0041,
    PC_GEM_BLUE             = 0x0042,
    PC_GEM_PURPLE           = 0x0043,
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
    PC_MODIFIER_HOOK        = 0x0112,
    PC_MODIFIER_H_POLE      = 0x0113,
    PC_MODIFIER_V_POLE      = 0x0114,
    PC_SCENERY_DESTRUCT     = 0x0116,
    PC_SCENERY_BUTTSTOMP    = 0x0117,
    PC_MODIFIER_HURT        = 0x0115,
    PC_LIGHT_SET            = 0x0120,
    PC_LIGHT_RESET          = 0x0124,
    PC_ENEMY_TURTLE_NORMAL  = 0x0180,
    PC_ENEMY_LIZARD         = 0x0185,
    PC_PUSHABLE_ROCK        = 0x00C5,
    PC_TRIGGER_CRATE        = 0x0060,
    PC_TRIGGER_AREA         = 0x011A,
    PC_MODIFIER_RICOCHET    = 0x0145,
    PC_BRIDGE               = 0x00C3,
    PC_AREA_STOP_ENEMY      = 0x0143,
    PC_COIN_SILVER          = 0x0048,
    PC_COIN_GOLD            = 0x0049,
    PC_MOVING_PLATFORM      = 0x00C4,
    PC_AREA_FLOAT_UP        = 0x0146,
    PC_MODIFIER_TUBE        = 0x0144,
    PC_FOOD_APPLE           = 0x008D,
    PC_FOOD_BANANA          = 0x008E,
    PC_FOOD_CHERRY          = 0x008F,
    PC_FOOD_ORANGE          = 0x0090,
    PC_FOOD_PEAR            = 0x0091,
    PC_FOOD_PRETZEL         = 0x0092,
    PC_FOOD_STRAWBERRY      = 0x0093,
    PC_FOOD_LEMON           = 0x009A,
    PC_FOOD_LIME            = 0x009B,
    PC_FOOD_THING           = 0x009C,
    PC_FOOD_WATERMELON      = 0x009D,
    PC_FOOD_PEACH           = 0x009E,
    PC_FOOD_GRAPES          = 0x009F,
    PC_FOOD_LETTUCE         = 0x00A0,
    PC_FOOD_EGGPLANT        = 0x00A1,
    PC_FOOD_CUCUMBER        = 0x00A2,
    PC_FOOD_PEPSI           = 0x00A3,
    PC_FOOD_COKE            = 0x00A4,
    PC_FOOD_MILK            = 0x00A5,
    PC_FOOD_PIE             = 0x00A6,
    PC_FOOD_CAKE            = 0x00A7,
    PC_FOOD_DONUT           = 0x00A8,
    PC_FOOD_CUPCAKE         = 0x00A9,
    PC_FOOD_CHIPS           = 0x00AA,
    PC_FOOD_CANDY           = 0x00AB,
    PC_FOOD_CHOCOLATE       = 0x00AC,
    PC_FOOD_ICE_CREAM       = 0x00AD,
    PC_FOOD_BURGER          = 0x00AE,
    PC_FOOD_PIZZA           = 0x00AF,
    PC_FOOD_FRIES           = 0x00B0,
    PC_FOOD_CHICKEN_LEG     = 0x00B1,
    PC_FOOD_SANDWICH        = 0x00B2,
    PC_FOOD_TACO            = 0x00B3,
    PC_FOOD_HOT_DOG         = 0x00B4,
    PC_FOOD_HAM             = 0x00B5,
    PC_FOOD_CHEESE          = 0x00B6,
    PC_SCENERY_DESTRUCT_SPD = 0x0118,
    PC_SCENERY_COLLAPSE     = 0x0119,
    PC_CARROT               = 0x0050,
    PC_TURTLE_SHELL         = 0x0182,
    PC_CARROT_FULL          = 0x0051,
    PC_WARP_COIN_BONUS      = 0x010D,
    PC_ENEMY_SUCKER         = 0x018C,
    PC_ENEMY_SUCKER_FLOAT   = 0x018D,
    PC_ENEMY_LAB_RAT        = 0x018B,
    PC_LIGHT_STEADY         = 0x0121,
    PC_LIGHT_PULSE          = 0x0122,
    PC_AREA_TEXT            = 0x014A,
    PC_CRATE_AMMO           = 0x0063,
};