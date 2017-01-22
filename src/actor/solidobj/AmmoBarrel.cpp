#include "AmmoBarrel.h"
#include "../weapon/Ammo.h"
#include "../Player.h"

AmmoBarrel::AmmoBarrel(const ActorInstantiationDetails& initData, WeaponType type)
    : BarrelContainer(initData) {
    if (type != WEAPON_BLASTER) {
        generateContents(eventFromType(type), 5);
    }
}

AmmoBarrel::~AmmoBarrel() {

}

void AmmoBarrel::handleCollision(std::shared_ptr<CommonActor> other) {
    BarrelContainer::handleCollision(other);

    if (contents.length() == 0) {
        std::shared_ptr<Ammo> ammo = std::dynamic_pointer_cast<Ammo>(other);
        if (ammo != nullptr && ammo->getType() != WEAPON_FREEZER) {
            auto types = generateRandomAmmo(ammo->getOwner().lock()->getAvailableWeaponTypes());
            for (auto type : types) {
                generateContents(eventFromType(type), 1);
            }
        }
    }
}
