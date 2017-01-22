#include "PowerUpMonitor.h"

#include "../weapon/Ammo.h"

PowerUpMonitor::PowerUpMonitor(const ActorInstantiationDetails& initData, WeaponType type)
    : SolidObject(initData, true), type(type) {
    loadResources("Object/PowerUpMonitor");

    setAnimation(AnimState::IDLE);

    switch (type) {
        case WEAPON_BLASTER: AnimationUser::setAnimation("OBJECT_POWER_UP_MONITOR_BLASTER_JAZZ"); break;
        case WEAPON_BOUNCER: AnimationUser::setAnimation("OBJECT_POWER_UP_MONITOR_BOUNCER");      break;
        case WEAPON_FREEZER: AnimationUser::setAnimation("OBJECT_POWER_UP_MONITOR_FREEZER");      break;
        case WEAPON_SEEKER:  AnimationUser::setAnimation("OBJECT_POWER_UP_MONITOR_SEEKER");       break;
        case WEAPON_RF:      AnimationUser::setAnimation("OBJECT_POWER_UP_MONITOR_RF");           break;
        case WEAPON_TOASTER: AnimationUser::setAnimation("OBJECT_POWER_UP_MONITOR_TOASTER");      break;
        case WEAPON_PEPPER:  AnimationUser::setAnimation("OBJECT_POWER_UP_MONITOR_PEPPER");       break;
        case WEAPON_ELECTRO: AnimationUser::setAnimation("OBJECT_POWER_UP_MONITOR_ELECTRO");      break;
        default:
            break;
    }
}

PowerUpMonitor::~PowerUpMonitor() {

}

void PowerUpMonitor::handleCollision(std::shared_ptr<CommonActor> other) {
    if (health == 0) {
        return;
    }

    auto ammo = std::dynamic_pointer_cast<Ammo>(other);
    if (ammo != nullptr) {
        if (ammo->getType() != WEAPON_FREEZER) {
            auto playerPtr = ammo->getOwner().lock();
            if (playerPtr != nullptr) {
                destroyAndApplyToPlayer(playerPtr, ammo->getStrength());
            }
        }
    }

    CommonActor::handleCollision(other);
}

bool PowerUpMonitor::perish() {
    return CommonActor::perish();
}

void PowerUpMonitor::destroyAndApplyToPlayer(std::shared_ptr<Player> player, int strength) {
    player->setPowerUp(type);
    decreaseHealth(strength);
    playSound("OBJECT_POWER_UP_DESTROY");
}
