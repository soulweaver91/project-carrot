#include "SavePoint.h"
#include "../gamestate/ActorAPI.h"

SavePoint::SavePoint(const ActorInstantiationDetails& initData) : CommonActor(initData), activated(false) {
    canBeFrozen = false;
    loadResources("Object/SavePoint");
    AnimationUser::setAnimation("OBJECT_SAVEPOINT_CLOSED");
}

SavePoint::~SavePoint() {

}

void SavePoint::tickEvent() {
    CommonActor::tickEvent();
}

void SavePoint::activateSavePoint() {
    if (!activated) {
        AnimationUser::setAnimation("OBJECT_SAVEPOINT_OPENED");
        setTransition(AnimState::TRANSITION_ACTIVATE, false);
        api->setSavePoint();
        playSound("OBJECT_SAVEPOINT_ACTIVATED");
        activated = true;
    }
}

void SavePoint::updateHitbox() {
    CommonActor::updateHitbox(20u, 20u);
}
