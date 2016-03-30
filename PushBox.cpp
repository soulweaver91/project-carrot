#include <memory>
#include "PushBox.h"
#include "SolidObject.h"
#include "CommonActor.h"
#include "CarrotQt5.h"

PushBox::PushBox(std::shared_ptr<CarrotQt5> root, double x, double y, int type) : SolidObject(root, x, y, true) {
    // Type: either PC_PUSHABLE_BOX or PC_PUSHABLE_ROCK, the only difference is the sprite
    addAnimation(AnimState::IDLE, "object/pushbox_rock.png",1,1,65,52,1,32,52);
    setAnimation(AnimState::IDLE);
}

PushBox::~PushBox() {

}
