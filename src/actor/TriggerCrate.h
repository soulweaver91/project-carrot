#pragma once

#include <memory>

#include "SolidObject.h"

class ActorAPI;

class TriggerCrate : public SolidObject {
public:
    TriggerCrate(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0, int triggerID = 0);
    ~TriggerCrate();
    bool perish() override;

private:
    int triggerID;
};
