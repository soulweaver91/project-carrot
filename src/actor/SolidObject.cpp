#include "SolidObject.h"
#include "../CarrotQt5.h"

SolidObject::SolidObject(std::shared_ptr<CarrotQt5> root, double x, double y, bool movable) 
    : CommonActor(root, x, y), movable(movable), isOneWay(false) {
}

SolidObject::~SolidObject() {

}

void SolidObject::push(bool left) {
    if (movable) {
        if (root->isPositionEmpty(getHitbox().add((left ? -1 : 1), 0), false, shared_from_this())) {
            posX += 0.6 * (left ? -1 : 1);
        }
    }
}

bool SolidObject::getIsOneWay() {
    return isOneWay;
}

bool SolidObject::perish() {
    // by default, solid objects cannot be destroyed
    // technically they all can be damaged but their health is simply ignored
    return false;
}
