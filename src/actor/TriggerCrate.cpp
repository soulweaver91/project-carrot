#include "TriggerCrate.h"
#include "../gamestate/ActorAPI.h"
#include "../gamestate/TileMap.h"

TriggerCrate::TriggerCrate(std::shared_ptr<ActorAPI> api, double x, double y, int triggerID)
    : SolidObject(api, x, y, true), triggerID(triggerID) {
    loadResources("Object/TriggerCrate");
    setAnimation(AnimState::IDLE);
}

TriggerCrate::~TriggerCrate() {
}

bool TriggerCrate::perish() {

    // crate can be destroyed
    if (health == 0) {
        auto tiles = api->getGameTiles().lock();

        if (tiles != nullptr) {
            tiles->setTrigger(triggerID, true);
        }

        playSound("OBJECT_TRIGGER_CRATE_DESTROY");
    }
    return CommonActor::perish();
}
