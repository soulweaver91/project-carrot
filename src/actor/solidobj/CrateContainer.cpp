#include "CrateContainer.h"
#include "../weapon/Ammo.h"

CrateContainer::CrateContainer(const ActorInstantiationDetails& initData) : GenericContainer(initData) {
    loadResources("Object/CrateContainer");
    AnimationUser::setAnimation("OBJECT_CRATE_GENERIC");
}

CrateContainer::CrateContainer(const ActorInstantiationDetails& initData, PCEvent type, uint count)
    : CrateContainer(initData) {
    generateContents(type, count);
}

CrateContainer::~CrateContainer() {
}

bool CrateContainer::perish() {
    if (health == 0) {
        playSound("OBJECT_CRATE_DESTROY");
    }
    return GenericContainer::perish();
}

void CrateContainer::handleCollision(std::shared_ptr<CommonActor> other) {
    CommonActor::handleCollision(other);

    std::shared_ptr<Ammo> ammo = std::dynamic_pointer_cast<Ammo>(other);
    if (ammo != nullptr) {
        decreaseHealth(ammo->getStrength());
    }
}
