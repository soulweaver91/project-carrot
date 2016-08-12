#pragma once

#include <memory>

#include "CommonActor.h"

class CarrotQt5;

class FrozenBlock : public CommonActor {
public:
    FrozenBlock(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0);
    ~FrozenBlock();
    bool perish() override;

private:
    uint ttl;
};
