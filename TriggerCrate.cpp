#include "TriggerCrate.h"
#include "SolidObject.h"
#include "CommonActor.h"
#include "CarrotQt5.h"

TriggerCrate::TriggerCrate(CarrotQt5* root, double x, double y, int trigger_id) : SolidObject(root, x, y, true), trigger_id(trigger_id) {
    addAnimation(AnimState::IDLE, "object/trigger_crate.png",1,1,28,34,1,14,34);
    setAnimation(AnimState::IDLE);
}

TriggerCrate::~TriggerCrate() {
}

bool TriggerCrate::perish() {
    // crate can be destroyed
    if (health == 0) {
        root->game_tiles->setTrigger(trigger_id,true);
    }
    return CommonActor::perish();
}
