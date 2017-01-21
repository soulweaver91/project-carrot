#include "BonusWarp.h"

#include "../gamestate/ActorAPI.h"
#include "../gamestate/EventMap.h"

BonusWarp::BonusWarp(std::shared_ptr<ActorAPI> api, double x, double y, const quint16 params[8])
: CommonActor(api, x, y, true) {
    canBeFrozen = false;
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
    auto events = api->getGameEvents().lock();
    if (events == nullptr) {
        return CoordinatePair(0, 0);
    }

    return events->getWarpTarget(eventParams[0]);
}

quint16 BonusWarp::getCost() {
    return eventParams[3];
}
