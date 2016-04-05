#include "SoundSystem.h"

SoundSystem::SoundSystem() : initialized(false) {
    // Attempt to start up the sound system, using the default audio device
    if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
        return;
    }

    BASS_SetVolume(1.0);
    initialized = true;
}

SoundSystem::~SoundSystem() {
    QList< HSAMPLE > samples = effect_bank.values();
    for (HSAMPLE sample : samples) {
        BASS_SampleFree(sample);
    }
}

void SoundSystem::playSFX(HSAMPLE sample) {
    HCHANNEL ch = BASS_SampleGetChannel(sample, false);
    if (ch != NULL) {
        BASS_ChannelPlay(ch, false);
    }
}

HSAMPLE SoundSystem::addSFX(const QString& id, const QString& path) {
    HSAMPLE nsample = BASS_SampleLoad(false, ("Data/Assets/" + path).toUtf8().data(), false, 0, 5, 0);
    if (nsample != 0) {
        effect_bank.insert(id, nsample);
    }

    return nsample;
}

void SoundSystem::fadeMusicOut(uint ms) {
    BASS_ChannelSlideAttribute(currentMusic, BASS_ATTRIB_MUSIC_VOL_GLOBAL, 0, ms);
}

void SoundSystem::fadeMusicIn(uint ms) {
    BASS_ChannelSlideAttribute(currentMusic, BASS_ATTRIB_MUSIC_VOL_GLOBAL, 128, ms);
}

bool SoundSystem::setMusic(const QString& filename) {
    // Stop the current music track and free its resources
    if (BASS_ChannelIsActive(currentMusic)) {
        BASS_ChannelStop(currentMusic);
        BASS_MusicFree(currentMusic);
    }

    // Load the new track and start playing it
    currentMusic = BASS_MusicLoad(false, filename.toUtf8().data(), 0, 0, BASS_SAMPLE_LOOP, 1);
    BASS_ChannelPlay(currentMusic, true);

    return true;
}

