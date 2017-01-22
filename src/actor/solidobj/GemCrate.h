#pragma once

#include "CrateContainer.h"

class GemCrate : public CrateContainer {
public:
    GemCrate(const ActorInstantiationDetails& initData, uint red, uint green, uint blue, uint purple);
    ~GemCrate();
};