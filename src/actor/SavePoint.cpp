#include "SavePoint.h"
#include "../CarrotQt5.h"

SavePoint::SavePoint(std::shared_ptr<CarrotQt5> root, double x, double y) : CommonActor(root, x, y, true), activated(false) {
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
        root->setSavePoint();
        playSound("OBJECT_SAVEPOINT_ACTIVATED");
        activated = true;
    }
}
