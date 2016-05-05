#pragma once

#include <memory>

#include "CommonActor.h"

class CarrotQt5;

class SavePoint : public CommonActor {
public:
    SavePoint(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0);
    ~SavePoint();
    void tickEvent();
    void activateSavePoint();
    Hitbox getHitbox() override;

private:
    bool activated;
};
