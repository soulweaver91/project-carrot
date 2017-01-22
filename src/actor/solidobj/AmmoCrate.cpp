#include "AmmoCrate.h"
#include "../weapon/Ammo.h"
#include "../Player.h"

AmmoCrate::AmmoCrate(const ActorInstantiationDetails& initData, WeaponType type)
    : CrateContainer(initData) {
    if (type != WEAPON_BLASTER) {
        generateContents(eventFromType(type), 5);
    }

    switch (type) {
        case WEAPON_BOUNCER: AnimationUser::setAnimation("OBJECT_CRATE_AMMO_BOUNCER"); break;
        case WEAPON_FREEZER: AnimationUser::setAnimation("OBJECT_CRATE_AMMO_FREEZER"); break;
        case WEAPON_SEEKER:  AnimationUser::setAnimation("OBJECT_CRATE_AMMO_SEEKER");  break;
        case WEAPON_RF:      AnimationUser::setAnimation("OBJECT_CRATE_AMMO_RF");      break;
        case WEAPON_TOASTER: AnimationUser::setAnimation("OBJECT_CRATE_AMMO_TOASTER"); break;
        case WEAPON_TNT:     AnimationUser::setAnimation("OBJECT_CRATE_AMMO_TNT");     break;
        default:
            break;
    }
}

AmmoCrate::~AmmoCrate() {

}

void AmmoCrate::handleCollision(std::shared_ptr<CommonActor> other) {
    CrateContainer::handleCollision(other);

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
