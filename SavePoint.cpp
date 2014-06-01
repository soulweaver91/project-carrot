#include "SavePoint.h"

SavePoint::SavePoint(CarrotQt5* root, double x, double y) : CommonActor(root, x, y, true), activated(false) {
    addAnimation(AnimState::IDLE,        "Data/Assets/savepoint.png",           1,1,42,70,1,19,69);
    addAnimation(AnimState::ACTIVATED,   "Data/Assets/savepoint_activated.png", 1,1,42,70,1,19,69);
    addAnimation(AnimState::TRANSITION_ACTIVATE, "Data/Assets/savepoint_activation.png", 15,1,42,70,10,19,69);

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
        root->sfxsys->playSFX(SFX_SAVE_POINT);
        activated = true;
    }
}
