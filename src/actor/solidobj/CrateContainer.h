#pragma once

#include "GenericContainer.h"

class CrateContainer : public GenericContainer {
public:
    CrateContainer(const ActorInstantiationDetails& initData);
    CrateContainer(const ActorInstantiationDetails& initData, PCEvent event, uint count);
    ~CrateContainer();
    bool perish() override;
    void handleCollision(std::shared_ptr<CommonActor> other) override;
};
