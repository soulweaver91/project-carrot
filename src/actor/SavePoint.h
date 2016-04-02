#pragma once

#include <memory>

#include "../CarrotQt5.h"
#include "CommonActor.h"

class SavePoint : public CommonActor {
public:
    SavePoint(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0);
    ~SavePoint();
    void tickEvent();
    void activateSavePoint();

private:
    bool activated;
};
