#pragma once

#include <memory>

#include "SolidObject.h"

class ActorAPI;

class TriggerCrate : public SolidObject {
public:
    TriggerCrate(const ActorInstantiationDetails& initData, int triggerID = 0);
    ~TriggerCrate();
    bool perish() override;

private:
    int triggerID;
};
