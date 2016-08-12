#include "FrozenBlock.h"
#include "../CarrotQt5.h"

FrozenBlock::FrozenBlock(std::shared_ptr<CarrotQt5> root, double x, double y)
    : CommonActor(root, x, y, false), ttl(3 * 70) {
    isGravityAffected = false;
    canBeFrozen = false;
    loadResources("Object/FrozenBlock");
    setAnimation(AnimState::IDLE);
}

FrozenBlock::~FrozenBlock() {
}

bool FrozenBlock::perish() {
    --ttl;
    if (ttl == 0) {
        health = 0;
    } else if (ttl == 35) {
        isBlinking = true;
    }

    return CommonActor::perish();
}
