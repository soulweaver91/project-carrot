#include "GemBarrel.h"

GemBarrel::GemBarrel(const ActorInstantiationDetails& initData, uint red, uint green, uint blue, uint purple)
    : BarrelContainer(initData) {
    generateContents(PC_GEM_RED, red);
    generateContents(PC_GEM_GREEN, green);
    generateContents(PC_GEM_BLUE, blue);
    generateContents(PC_GEM_PURPLE, purple);
}

GemBarrel::~GemBarrel() {

}
