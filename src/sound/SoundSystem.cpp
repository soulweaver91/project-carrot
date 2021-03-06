#include "SoundSystem.h"
#include "../actor/CommonActor.h"
#include <QList>
#include <cassert>

SoundSystem::SoundSystem() : initialized(false), isModMusic(false) {
    // Attempt to start up the sound system, using the default audio device
    if (!BASS_Init(-1, 44100, BASS_DEVICE_3D, 0, NULL) || !BASS_PluginLoad("bassfx.dll", 0)) {
        return;
    }

    BASS_SetVolume(1.0);
    BASS_FX_GetVersion();

    BASS_3DVECTOR zeroes = { 0, 0, 0 };
    BASS_Set3DPosition(&zeroes, NULL, NULL, NULL);
    BASS_Set3DFactors(1.0, 5.0, 0.0);
    BASS_Apply3D();

    initialized = true;
}

SoundSystem::~SoundSystem() {
    BASS_Free();
}

HCHANNEL SoundSystem::playSFX(HSAMPLE sample, bool use3D) {
    HCHANNEL ch = BASS_SampleGetChannel(sample, false);
    if (ch != 0) {
        if (!use3D) {
            BASS_ChannelSet3DAttributes(ch, BASS_3DMODE_OFF, 0, 0, 0, 0, 0);
            BASS_Apply3D();
        }

        BASS_ChannelPlay(ch, false);
    }
    return ch;
}

HCHANNEL SoundSystem::playSFX(HSAMPLE sample, bool use3D, float speed, float pitch, float freq) {
    BASS_SAMPLE params;
    if (!BASS_SampleGetInfo(sample, &params)) {
        return 0;
    }

    void* buffer = operator new(params.length);
    BASS_SampleGetData(sample, buffer);

    HSTREAM stream = BASS_StreamCreate(params.freq, params.chans, params.flags | BASS_STREAM_DECODE, STREAMPROC_PUSH, NULL);
    BASS_StreamPutData(stream, buffer, params.length | BASS_STREAMPROC_END);

    operator delete(buffer);

    HSTREAM ch = BASS_FX_TempoCreate(stream, BASS_STREAM_AUTOFREE | BASS_SAMPLE_3D);
    if (ch != 0) {
        if (!use3D) {
            BASS_ChannelSet3DAttributes(ch, BASS_3DMODE_OFF, 0, 0, 0, 0, 0);
            BASS_Apply3D();
        }

        BASS_ChannelSetAttribute(ch, BASS_ATTRIB_TEMPO, speed);
        BASS_ChannelSetAttribute(ch, BASS_ATTRIB_TEMPO_PITCH, pitch);
        BASS_ChannelSetAttribute(ch, BASS_ATTRIB_TEMPO_FREQ, freq * params.freq);
        BASS_ChannelPlay(ch, false);

    }

    return ch;
}

void SoundSystem::playSFX(HSAMPLE sample, const CoordinatePair& pos) {
    for (auto listener : listeners) {
        HCHANNEL ch = playSFX(sample, true);
        append3DSoundIfNotNull(listener, ch, pos);
    }
}

void SoundSystem::playSFX(HSAMPLE sample, const CoordinatePair& pos, float speed, float pitch, float freq) {
    for (auto listener : listeners) {
        HCHANNEL ch = playSFX(sample, true, speed, pitch, freq);
        append3DSoundIfNotNull(listener, ch, pos);
    }
}

void SoundSystem::append3DSoundIfNotNull(std::shared_ptr<SoundListener> listener, HCHANNEL ch, const CoordinatePair& pos) {
    if (ch != 0) {
        BASS_ChannelSet3DAttributes(ch, BASS_3DMODE_NORMAL, 5, 0, 360, 360, 1);
        listener->sounds.append(std::make_shared<SoundInstance>(ch, pos));
    }
}

HSAMPLE SoundSystem::addSFX(const QString& id, const QString& path) {
    HSAMPLE sampleID = BASS_SampleLoad(false, ("Data/Assets/" + path).toUtf8().data(), false, 0, 5, BASS_SAMPLE_3D);
    if (sampleID != 0) {
        effectBank.insert(id, sampleID);
    }

    return sampleID;
}

void SoundSystem::fadeMusicOut(uint ms) {
    BASS_ChannelSlideAttribute(currentMusic, BASS_ATTRIB_VOL, 0.0f, ms);
}

void SoundSystem::fadeMusicIn(uint ms) {
    BASS_ChannelSlideAttribute(currentMusic, BASS_ATTRIB_VOL, 1.0f, ms);
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

uint SoundSystem::registerSoundListener(std::weak_ptr<CommonActor> listener) {
    auto listenerObj = std::make_shared<SoundListener>(listener, nextListenerID);
    listeners.insert(nextListenerID, listenerObj);

    return nextListenerID++;
}

void SoundSystem::unregisterSoundListener(const uint listener) {
    listeners.remove(listener);
}

void SoundSystem::unregisterAllSoundListeners() {
    listeners.clear();
}

void SoundSystem::updateSoundPositions() {
    foreach (auto listener, listeners) {
        if (listener->listener.expired()) {
            unregisterSoundListener(listener->id);
            continue;
        }

        CoordinatePair coords = listener->listener.lock()->getPosition();
        for (auto soundInstance : listener->sounds) {
            if (BASS_ChannelIsActive(soundInstance->sound)) {
                CoordinatePair pos = coords - soundInstance->position;
                BASS_3DVECTOR spatialPos = { (float) - pos.x / TILE_WIDTH, (float) - pos.y / TILE_HEIGHT, 0 };
                BASS_ChannelSet3DPosition(soundInstance->sound, &spatialPos, NULL, NULL);
            }
        }
    }

    BASS_Apply3D();
}

bool SoundSystem::setMusic(const QString& filename) {
    // Stop the current music track and free its resources
    if (BASS_ChannelIsActive(currentMusic)) {
        BASS_ChannelStop(currentMusic);
        if (isModMusic) {
            BASS_MusicFree(currentMusic);
        } else {
            BASS_StreamFree(currentMusic);
        }
    }

    // Load the new track and start playing it
    currentMusic = BASS_MusicLoad(false, filename.toUtf8().data(), 0, 0, BASS_SAMPLE_LOOP, 1);
    if (currentMusic != 0) {
        BASS_ChannelPlay(currentMusic, true);
        isModMusic = true;
        return true;
    }

    currentMusic = BASS_StreamCreateFile(false, filename.toUtf8().data(), 0, 0, BASS_SAMPLE_LOOP);
    if (currentMusic != 0) {
        BASS_ChannelPlay(currentMusic, true);
        isModMusic = false;
        return true;
    }

    return false;
}

