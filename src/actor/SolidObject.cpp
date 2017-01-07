#include "SolidObject.h"
#include "../gamestate/ActorAPI.h"

SolidObject::SolidObject(std::shared_ptr<ActorAPI> api, double x, double y, bool movable)
    : CommonActor(api, x, y), movable(movable), isOneWay(false) {
}

SolidObject::~SolidObject() {

}

void SolidObject::push(bool left) {
    if (movable) {
        moveInstantly({ 0.3 * (left ? -1 : 1), 0.0 }, false);
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
