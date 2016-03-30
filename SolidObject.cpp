#include <memory>
#include "SolidObject.h"
#include "CommonActor.h"
#include "CarrotQt5.h"

SolidObject::SolidObject(std::shared_ptr<CarrotQt5> root, double x, double y, bool movable) : CommonActor(root, x, y), movable(movable), one_way(false) {
}

SolidObject::~SolidObject() {

}

void SolidObject::push(bool left) {
    if (movable) {
        if (root->isPositionEmpty(CarrotQt5::calcHitbox(getHitbox(),(left ? -1 : 1),0),false,this)) {
            pos_x += 0.6 * (left ? -1 : 1);
        }
    }
}

bool SolidObject::isOneWay() {
    return one_way;
}

bool SolidObject::perish() {
    // by default, solid objects cannot be destroyed
    // technically they all can be damaged but their health is simply ignored
    return false;
}
