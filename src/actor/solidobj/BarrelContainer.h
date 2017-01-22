#pragma once

#include "GenericContainer.h"

class BarrelContainer : public GenericContainer {
public:
    BarrelContainer(const ActorInstantiationDetails& initData);
    BarrelContainer(const ActorInstantiationDetails& initData, PCEvent event, uint count);
    ~BarrelContainer();
    bool perish() override;
};
