#include "TriggerCrate.h"
#include "../CarrotQt5.h"
#include "../gamestate/TileMap.h"

TriggerCrate::TriggerCrate(std::shared_ptr<CarrotQt5> root, double x, double y, int trigger_id) 
    : SolidObject(root, x, y, true), trigger_id(trigger_id) {
    loadResources("Object/TriggerCrate");
    setAnimation(AnimState::IDLE);
}

TriggerCrate::~TriggerCrate() {
}

bool TriggerCrate::perish() {

    // crate can be destroyed
    if (health == 0) {
        auto tiles = root->getGameTiles().lock();

        if (tiles != nullptr) {
            tiles->setTrigger(trigger_id, true);
        }
    }
    return CommonActor::perish();
}
