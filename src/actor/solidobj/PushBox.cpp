#include "PushBox.h"

PushBox::PushBox(const ActorInstantiationDetails& initData, int type) : SolidObject(initData, true) {
    loadResources("Object/PushBox");
    if (type == 0) {
        AnimationUser::setAnimation("OBJECT_PUSHBOX_ROCK");
    } else if (type == 1) {
        AnimationUser::setAnimation("OBJECT_PUSHBOX_CRATE");
    }
}

PushBox::~PushBox() {

}