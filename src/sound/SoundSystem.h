#pragma once

#include <bass.h>
#include <bass_fx.h>
#include <QString>
#include <QVector>
#include <QMultiMap>

struct SFXBinding {
    enum SFXType type;
    HSAMPLE effect;
};

class SoundSystem {
public:
    SoundSystem();
    ~SoundSystem();
    HCHANNEL playSFX(HSAMPLE sample);
    HCHANNEL playSFX(HSAMPLE sample, float speed, float pitch = 0.0, float freq = 1.0);
    HSAMPLE addSFX(const QString& id, const QString& path);
    bool setMusic(const QString& filename);
    void fadeMusicOut(uint ms);
    void fadeMusicIn(uint ms);
    void pauseMusic();
    void resumeMusic();
    void clearSounds();

private:
    QMultiMap<QString, HSAMPLE> effectBank;
    bool initialized;
    HMUSIC currentMusic;
};
