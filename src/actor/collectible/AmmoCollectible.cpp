#include "AmmoCollectible.h"
#include "../Player.h"

AmmoCollectible::AmmoCollectible(std::shared_ptr<CarrotQt5> root, WeaponType type, double x, double y, bool fromEventMap)
: Collectible(root, x, y, fromEventMap), weaponType(type) {
    scoreValue = 100;

    switch (weaponType) {
        case WEAPON_BOUNCER: AnimationUser::setAnimation("PICKUP_AMMO_BOUNCER"); break;
        case WEAPON_FREEZER: AnimationUser::setAnimation("PICKUP_AMMO_FREEZER"); break;
        case WEAPON_SEEKER:  AnimationUser::setAnimation("PICKUP_AMMO_SEEKER");  break;
        case WEAPON_RF:      AnimationUser::setAnimation("PICKUP_AMMO_RF");      break;
        case WEAPON_TOASTER: AnimationUser::setAnimation("PICKUP_AMMO_TOASTER"); break;
        case WEAPON_TNT:     AnimationUser::setAnimation("PICKUP_AMMO_TNT");     break;
        case WEAPON_PEPPER:  AnimationUser::setAnimation("PICKUP_AMMO_PEPPER");  break;
        case WEAPON_ELECTRO: AnimationUser::setAnimation("PICKUP_AMMO_ELECTRO"); break;
    }
}

void AmmoCollectible::collect(std::shared_ptr<Player> player) {
    player->addAmmo(weaponType, 3);
    Collectible::collect(player);
}
