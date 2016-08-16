#include "PushBox.h"

PushBox::PushBox(std::shared_ptr<ActorAPI> api, double x, double y, int type) : SolidObject(api, x, y, true) {
    loadResources("Object/PushBox");
    if (type == 0) {
        AnimationUser::setAnimation("OBJECT_PUSHBOX_ROCK");
    } else if (type == 1) {
        AnimationUser::setAnimation("OBJECT_PUSHBOX_CRATE");
    }
}

PushBox::~PushBox() {

}