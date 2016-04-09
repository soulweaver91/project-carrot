#pragma once

#include <memory>

#include "CommonActor.h"

class CarrotQt5;

class SolidObject : public CommonActor {
public:
    SolidObject(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0, bool movable = true);
    ~SolidObject();
    void push(bool left);
    bool getIsOneWay();
    virtual bool perish() override;

protected:
    bool movable;
    bool isOneWay;
};
