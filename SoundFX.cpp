#include <bass.h>
#include "SoundFX.h"

SFXSystem::SFXSystem() {

}

SFXSystem::~SFXSystem() {
    QList< HSAMPLE > samples = effect_bank.values();
    for (HSAMPLE sample : samples) {
        BASS_SampleFree(sample);
    }
}

void SFXSystem::playSFX(SFXType type, int idx) {
    QList< HSAMPLE > samples = effect_bank.values(type);
    if (!samples.empty()) {
        if ((idx == -1) || (idx >= samples.size())) {
            idx = qrand() % samples.size();
        } else {
            // QMultiMap returns items in descending order of insertion
            idx = samples.size() - (idx + 1);
        }
        HCHANNEL ch = BASS_SampleGetChannel(samples.at(idx),false);
        BASS_ChannelPlay(ch,false);
    }
}

bool SFXSystem::addSFX(SFXType type, const QString& path) {
    HSAMPLE nsample = BASS_SampleLoad(false,path.toUtf8().data(), false, 0, 5, 0);
    if (nsample != 0) {
        effect_bank.insert(type,nsample);
        return true;
    } else {
        return false;
    }
}
