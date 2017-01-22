#pragma once

#include "BarrelContainer.h"

class GemBarrel : public BarrelContainer {
public:
    GemBarrel(const ActorInstantiationDetails& initData, uint red, uint green, uint blue, uint purple);
    ~GemBarrel();
};