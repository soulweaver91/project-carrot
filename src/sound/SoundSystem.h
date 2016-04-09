#pragma once

#include <bass.h>
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
    void playSFX(HSAMPLE sample);
    HSAMPLE addSFX(const QString& id, const QString& path);
    bool setMusic(const QString& filename);
    void fadeMusicOut(uint ms);
    void fadeMusicIn(uint ms);

private:
    QMultiMap<QString, HSAMPLE> effectBank;
    bool initialized;
    HMUSIC currentMusic;
};
