#include "PushBox.h"

PushBox::PushBox(std::shared_ptr<CarrotQt5> root, double x, double y, int type) : SolidObject(root, x, y, true) {
    loadResources("Object/PushBox");
    // Type: either PC_PUSHABLE_BOX or PC_PUSHABLE_ROCK, the only difference is the sprite
    if (static_cast<PCEvent>(type) == PCEvent::PC_PUSHABLE_ROCK) {
        AnimationUser::setAnimation("OBJECT_PUSHBOX_ROCK");
    }
}

PushBox::~PushBox() {

}