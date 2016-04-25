#include "SoundSystem.h"
#include <QList>
#include <cassert>

SoundSystem::SoundSystem() : initialized(false) {
    // Attempt to start up the sound system, using the default audio device
    if (!BASS_Init(-1, 44100, 0, 0, NULL) || !BASS_PluginLoad("bassfx.dll", 0)) {
        return;
    }

    BASS_SetVolume(1.0);
    BASS_FX_GetVersion();
    initialized = true;
}

SoundSystem::~SoundSystem() {
    BASS_Free();
}

HCHANNEL SoundSystem::playSFX(HSAMPLE sample) {
    HCHANNEL ch = BASS_SampleGetChannel(sample, false);
    if (ch != NULL) {
        BASS_ChannelPlay(ch, false);
    }
    return ch;
}

HSTREAM SoundSystem::playSFX(HSAMPLE sample, float speed, float pitch, float freq) {
    BASS_SAMPLE params;
    if (!BASS_SampleGetInfo(sample, &params)) {
        return NULL;
    }

    void* buffer = operator new(params.length);
    BASS_SampleGetData(sample, buffer);

    HSTREAM stream = BASS_StreamCreate(params.freq, params.chans, params.flags | BASS_STREAM_DECODE, STREAMPROC_PUSH, NULL);
    BASS_StreamPutData(stream, buffer, params.length | BASS_STREAMPROC_END);
    operator delete(buffer);

    HSTREAM ch = BASS_FX_TempoCreate(stream, BASS_STREAM_AUTOFREE);
    if (ch != NULL) {
        BASS_ChannelSetAttribute(ch, BASS_ATTRIB_TEMPO, speed);
        BASS_ChannelSetAttribute(ch, BASS_ATTRIB_TEMPO_PITCH, pitch);
        BASS_ChannelSetAttribute(ch, BASS_ATTRIB_TEMPO_FREQ, freq * params.freq);
        BASS_ChannelPlay(ch, false);
    }
    return ch;
}

HSAMPLE SoundSystem::addSFX(const QString& id, const QString& path) {
    HSAMPLE sampleID = BASS_SampleLoad(false, ("Data/Assets/" + path).toUtf8().data(), false, 0, 5, 0);
    if (sampleID != 0) {
        effectBank.insert(id, sampleID);
    }

    return sampleID;
}

void SoundSystem::fadeMusicOut(uint ms) {
    BASS_ChannelSlideAttribute(currentMusic, BASS_ATTRIB_MUSIC_VOL_GLOBAL, 0, ms);
}

void SoundSystem::fadeMusicIn(uint ms) {
    BASS_ChannelSlideAttribute(currentMusic, BASS_ATTRIB_MUSIC_VOL_GLOBAL, 128, ms);
}

void SoundSystem::pauseMusic() {
    if (BASS_ChannelIsActive(currentMusic)) {
        BASS_ChannelPause(currentMusic);
    }
}

void SoundSystem::resumeMusic() {
    if (BASS_ChannelIsActive(currentMusic)) {
        BASS_ChannelPlay(currentMusic, false);
    }
}

void SoundSystem::clearSounds() {
    BASS_Stop();
    BASS_Start();
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

