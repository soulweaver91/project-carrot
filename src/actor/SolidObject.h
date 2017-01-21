#pragma once

#include <memory>

#include "CommonActor.h"

class ActorAPI;

class SolidObject : public CommonActor {
public:
    SolidObject(const ActorInstantiationDetails& initData, bool movable = true);
    ~SolidObject();
    void push(bool left);
    bool getIsOneWay();
    virtual bool perish() override;

protected:
    bool movable;
    bool isOneWay;
};
