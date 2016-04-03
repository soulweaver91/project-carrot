#include "SavePoint.h"
#include "../CarrotQt5.h"

SavePoint::SavePoint(std::shared_ptr<CarrotQt5> root, double x, double y) : CommonActor(root, x, y, true), activated(false) {
    addAnimation(AnimState::IDLE,        "object/savepoint.png",           1,1,42,70,1,19,69);
    addAnimation(AnimState::ACTIVATED,   "object/savepoint.png", 1,1,42,70,1,19,69);
    addAnimation(AnimState::TRANSITION_ACTIVATE, "object/savepoint.png", 15,1,42,70,10,19,69);

    setAnimation(AnimState::IDLE);
}

SavePoint::~SavePoint() {

}

void SavePoint::tickEvent() {
    CommonActor::tickEvent();
}

void SavePoint::activateSavePoint() {
    if (!activated) {
        setAnimation(AnimState::ACTIVATED);
        setTransition(AnimState::TRANSITION_ACTIVATE, false);
        root->setSavePoint();
        playSound(SFX_SAVE_POINT);
        activated = true;
    }
}
