#include <memory>
#include "Collectible.h"
#include "../struct/Constants.h"

Collectible::Collectible(std::shared_ptr<CarrotQt5> root, enum CollectibleType type, double x, double y, bool fromEventMap)
    : CommonActor(root, x, y, fromEventMap), type(type), untouched(true) {
    phase = ((x / 100.0) + (y / 100.0));
    elasticity = 0.6;

    loadResources("Object/Collectible");
    // temporary code
    switch(type) {
        case COLLTYPE_FAST_FIRE:    AnimationUser::setAnimation("PICKUP_FASTFIRE"); break;
        case COLLTYPE_AMMO_BOUNCER: AnimationUser::setAnimation("PICKUP_AMMO_BOUNCER"); break;
        case COLLTYPE_AMMO_FREEZER: AnimationUser::setAnimation("PICKUP_AMMO_FREEZER"); break;
        case COLLTYPE_AMMO_SEEKER:  AnimationUser::setAnimation("PICKUP_AMMO_SEEKER"); break;
        case COLLTYPE_AMMO_RF:      AnimationUser::setAnimation("PICKUP_AMMO_RF"); break;
        case COLLTYPE_AMMO_TOASTER: AnimationUser::setAnimation("PICKUP_AMMO_TOASTER"); break;
        case COLLTYPE_AMMO_TNT:     AnimationUser::setAnimation("PICKUP_AMMO_TNT"); break;
        case COLLTYPE_AMMO_PEPPER:  AnimationUser::setAnimation("PICKUP_AMMO_PEPPER"); break;
        case COLLTYPE_AMMO_ELECTRO: AnimationUser::setAnimation("PICKUP_AMMO_ELECTRO"); break;
        case COLLTYPE_GEM_RED:      
            AnimationUser::setAnimation("PICKUP_GEM");
            currentAnimation.setColor({ 511, 0, 0 });
            break;
        case COLLTYPE_GEM_GREEN:    
            AnimationUser::setAnimation("PICKUP_GEM");
            currentAnimation.setColor({ 0, 511, 0 });
            break;
        case COLLTYPE_GEM_BLUE:     
            AnimationUser::setAnimation("PICKUP_GEM");
            currentAnimation.setColor({ 0, 0, 511 });
            break;
        case COLLTYPE_COIN_GOLD:    AnimationUser::setAnimation("PICKUP_COIN_GOLD"); break;
        case COLLTYPE_COIN_SILVER:  AnimationUser::setAnimation("PICKUP_COIN_SILVER"); break;
    }
    setFacingDirection();
    setAnimation(AnimState::IDLE);
}

Collectible::~Collectible() {

}

void Collectible::tickEvent() {
    if (!untouched) {
        // Do not apply default movement if we haven't been shot yet
        CommonActor::tickEvent();
    }
}

void Collectible::impact(double forceX, double forceY) {
    if (untouched) {
        externalForceX += forceX * (0.9 + (qrand() % 2000) / 10000.0);
        externalForceY += forceY * (0.9 + (qrand() % 2000) / 10000.0);
    }
    untouched = false;
}

void Collectible::drawUpdate() {
    double waveOffset = 2.4 * cos((phase * 0.3) * PI);

    // The position of the actor is altered for the draw event.
    // We want to keep the actual position of the actor constant, though.
    posY += waveOffset;
    CommonActor::drawUpdate();
    posY -= waveOffset;
    phase += 0.1;
}

void Collectible::setFacingDirection() {
    switch (type) {
        case COLLTYPE_FAST_FIRE:
        case COLLTYPE_AMMO_TOASTER:
        case COLLTYPE_AMMO_BOUNCER:
        case COLLTYPE_AMMO_SEEKER:
            return;
        default:
            if ((qRound(posX + posY) / 32) % 2 == 1) {
                isFacingLeft = true;
            }
    }
}
