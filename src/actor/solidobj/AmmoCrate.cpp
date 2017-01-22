#include "AmmoCrate.h"

AmmoCrate::AmmoCrate(const ActorInstantiationDetails& initData, WeaponType type)
    : CrateContainer(initData) {
    if (type != WEAPON_BLASTER) {
        generateContents((PCEvent)((int)PC_AMMO_BOUNCER + (int)type - 1), 5);
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
