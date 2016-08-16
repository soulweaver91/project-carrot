#pragma once

#include <memory>

#include "CommonActor.h"

class ActorAPI;

class SolidObject : public CommonActor {
public:
    SolidObject(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0, bool movable = true);
    ~SolidObject();
    void push(bool left);
    bool getIsOneWay();
    virtual bool perish() override;

protected:
    bool movable;
    bool isOneWay;
};
