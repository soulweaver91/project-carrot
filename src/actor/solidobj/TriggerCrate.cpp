#include "TriggerCrate.h"
#include "../../gamestate/ActorAPI.h"
#include "../../gamestate/TileMap.h"

TriggerCrate::TriggerCrate(const ActorInstantiationDetails& initData, int triggerID)
    : SolidObject(initData, true), triggerID(triggerID) {
    loadResources("Object/TriggerCrate");
    setAnimation(AnimState::IDLE);
}

TriggerCrate::~TriggerCrate() {
}

bool TriggerCrate::perish() {

    // crate can be destroyed
    if (health == 0) {
        api->setTrigger(triggerID, true);
        playSound("OBJECT_TRIGGER_CRATE_DESTROY");
    }
    return CommonActor::perish();
}
