#pragma once

#include <bass.h>
#include <bass_fx.h>
#include <memory>
#include <QString>
#include <QVector>
#include <QMultiMap>
#include "../struct/CoordinatePair.h"

class CommonActor;

struct SoundInstance {
    const HCHANNEL sound;
    const CoordinatePair position;

    SoundInstance(const HCHANNEL sound, const CoordinatePair position) :sound(sound), position(position) {

    }
};

struct SoundListener {
    const std::weak_ptr<CommonActor> listener;

    // Saved redundantly for easier self-unregistration.
    const uint id;
    QVector<std::shared_ptr<SoundInstance>> sounds;

    SoundListener(const std::weak_ptr<CommonActor> listener, const uint id) : listener(listener), id(id) {

    }
};

class SoundSystem {
public:
    SoundSystem();
    ~SoundSystem();
    HCHANNEL playSFX(HSAMPLE sample, bool use3D);
    HCHANNEL playSFX(HSAMPLE sample, bool use3D, float speed, float pitch = 0.0, float freq = 1.0);
    void playSFX(HSAMPLE sample, const CoordinatePair& pos);
    void playSFX(HSAMPLE sample, const CoordinatePair& pos, float speed, float pitch = 0.0, float freq = 1.0);
    HSAMPLE addSFX(const QString& id, const QString& path);
    bool setMusic(const QString& filename);
    void fadeMusicOut(uint ms);
    void fadeMusicIn(uint ms);
    void pauseMusic();
    void resumeMusic();
    void clearSounds();
    uint registerSoundListener(std::weak_ptr<CommonActor> listener);
    void unregisterSoundListener(const uint listener);
    void unregisterAllSoundListeners();
    void updateSoundPositions();

private:
    void append3DSoundIfNotNull(std::shared_ptr<SoundListener> listener, HCHANNEL ch, const CoordinatePair& pos);
    QMultiMap<QString, HSAMPLE> effectBank;
    QMap<uint, std::shared_ptr<SoundListener>> listeners;
    uint nextListenerID;
    bool initialized;
    HMUSIC currentMusic;
};
