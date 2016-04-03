#pragma once

#include <bass.h>
#include <QString>
#include <QList>
#include <QMultiMap>

enum SFXType {
    SFX_BLASTER_SHOOT_JAZZ,
    SFX_TOASTER_SHOOT,
    SFX_AMMO_HIT_WALL,
    SFX_JUMP,
    SFX_LAND,
    SFX_COLLECT_AMMO,
    SFX_COLLECT_GEM,
    SFX_COLLECT_COIN,
    SFX_COLLECT_1UP,
    SFX_COLLECT_FOOD,
    SFX_COLLECT_DRINK,
    SFX_SWITCH_AMMO,
    SFX_JAZZ_HURT,
    SFX_JAZZ_IDLE,
    SFX_SAVE_POINT,
    SFX_JAZZ_EOL,
    SFX_WARP_IN,
    SFX_WARP_OUT,
    SFX_BLOCK_DESTRUCT,
    SFX_LIZARD_SPONTANEOUS
};

struct SFXBinding {
    enum SFXType type;
    HSAMPLE effect;
};

class SoundSystem {
    public:
        SoundSystem();
        ~SoundSystem();
        void playSFX(SFXType type, int idx = -1);
        bool addSFX(SFXType type, const QString& path);
        bool setMusic(const QString& filename);
        void fadeMusicOut(uint ms);
        void fadeMusicIn(uint ms);
    private:
        QMultiMap< SFXType, HSAMPLE > effect_bank;
        bool initialized;
        HMUSIC currentMusic;
};
