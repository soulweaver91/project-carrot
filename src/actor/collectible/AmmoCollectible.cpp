#include "AmmoCollectible.h"
#include "../Player.h"
#include "../../gamestate/GameView.h"

AmmoCollectible::AmmoCollectible(const ActorInstantiationDetails& initData, WeaponType type, bool fromEventMap)
: Collectible(initData, fromEventMap), weaponType(type) {
    scoreValue = 100;

    QString spriteName = "";
    switch (weaponType) {
        case WEAPON_BOUNCER: spriteName = "BOUNCER"; break;
        case WEAPON_FREEZER: spriteName = "FREEZER"; break;
        case WEAPON_SEEKER:  spriteName = "SEEKER";  break;
        case WEAPON_RF:      spriteName = "RF";      break;
        case WEAPON_TOASTER: spriteName = "TOASTER"; break;
        case WEAPON_TNT:     spriteName = "TNT";     break;
        case WEAPON_PEPPER:  spriteName = "PEPPER";  break;
        case WEAPON_ELECTRO: spriteName = "ELECTRO"; break;
        default:
            break;
    }
    
    normalSprite = std::make_shared<AnimationInstance>(this);
    auto res = animationBank.value("PICKUP_AMMO_" + spriteName);
    if (res != nullptr) {
        normalSprite->setAnimation(res);
    }

    if (spriteName != "TNT") {
        spriteName = "POWERUP_" + spriteName;
    }

    poweredUpSprite = std::make_shared<AnimationInstance>(this);
    res = animationBank.value("PICKUP_AMMO_" + spriteName);
    if (res != nullptr) {
        poweredUpSprite->setAnimation(res);
    }
}

void AmmoCollectible::collect(std::shared_ptr<Player> player) {
    player->addAmmo(weaponType, 3);
    Collectible::collect(player);
}

void AmmoCollectible::drawUpdate(std::shared_ptr<GameView>& view) {
    auto player = view->getViewPlayer().lock();
    currentAnimation = (player != nullptr && player->getPowerUp(weaponType) ? poweredUpSprite : normalSprite);

    CommonActor::drawUpdate(view);
}
