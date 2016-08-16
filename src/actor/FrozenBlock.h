#pragma once

#include <memory>

#include "CommonActor.h"

class ActorAPI;

class FrozenBlock : public CommonActor {
public:
    FrozenBlock(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0);
    ~FrozenBlock();
    bool perish() override;

private:
    uint ttl;
};
