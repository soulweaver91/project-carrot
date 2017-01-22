#include "BarrelContainer.h"
#include "../weapon/Ammo.h"

BarrelContainer::BarrelContainer(const ActorInstantiationDetails& initData) : GenericContainer(initData) {
    loadResources("Object/BarrelContainer");
    AnimationUser::setAnimation("OBJECT_BARREL_GENERIC");
}

BarrelContainer::BarrelContainer(const ActorInstantiationDetails& initData, PCEvent type, uint count)
    : BarrelContainer(initData) {
    generateContents(type, count);
}

BarrelContainer::~BarrelContainer() {
}

bool BarrelContainer::perish() {
    if (health == 0) {
        playSound("OBJECT_BARREL_DESTROY");
    }
    return GenericContainer::perish();
}
