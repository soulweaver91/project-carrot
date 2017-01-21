#pragma once

#include <memory>

#include "CommonActor.h"

class ActorAPI;

class FrozenBlock : public CommonActor {
public:
    FrozenBlock(const ActorInstantiationDetails& initData);
    ~FrozenBlock();
    bool perish() override;

private:
    uint ttl;
};
