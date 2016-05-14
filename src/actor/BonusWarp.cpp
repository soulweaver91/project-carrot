#include "BonusWarp.h"

#include "../CarrotQt5.h"
#include "../gamestate/EventMap.h"

BonusWarp::BonusWarp(std::shared_ptr<CarrotQt5> root, double x, double y, const quint16 params[8])
: CommonActor(root, x, y, true) {
    std::copy_n(params, 8, eventParams);
    loadResources("Object/BonusWarp");

    switch (eventParams[3]) {
        case 10:
            AnimationUser::setAnimation("OBJECT_WARP_BONUS_10");
            break;
        case 20:
            AnimationUser::setAnimation("OBJECT_WARP_BONUS_20");
            break;
        case 50:
            AnimationUser::setAnimation("OBJECT_WARP_BONUS_50");
            break;
        case 100:
            AnimationUser::setAnimation("OBJECT_WARP_BONUS_100");
            break;
        default:
            // TODO: ping-pong animation not supported for actors at present
            AnimationUser::setAnimation("OBJECT_WARP_BONUS_GENERIC");
    }
}

BonusWarp::~BonusWarp() {

}

const CoordinatePair BonusWarp::getWarpTarget() {
    auto events = root->getGameEvents().lock();
    if (events == nullptr) {
        return CoordinatePair(0, 0);
    }

    return events->getWarpTarget(eventParams[0]);
}

void BonusWarp::getParams(quint16 (&params)[8]) {
    std::copy_n(eventParams, 8, params);
}
