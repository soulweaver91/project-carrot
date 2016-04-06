#include <memory>
#include "Collectible.h"

#define PI 3.1415926535

Collectible::Collectible(std::shared_ptr<CarrotQt5> root, enum CollectibleType type, double x, double y, bool fromEventMap)
    : CommonActor(root, x, y, fromEventMap), type(type), intact(true) {
    phase = ((x / 100.0) + (y / 100.0));
    elasticity = 0.6;

    loadResources("Object/Collectible");
    // temporary code
    switch(type) {
        case COLLTYPE_FAST_FIRE:    AnimationUser::setAnimation("PICKUP_FASTFIRE"); break;
        case COLLTYPE_AMMO_TOASTER: AnimationUser::setAnimation("PICKUP_AMMO_TOASTER"); break;
        case COLLTYPE_AMMO_BOUNCER: AnimationUser::setAnimation("PICKUP_AMMO_BOUNCER"); break;
        case COLLTYPE_AMMO_SEEKER:  AnimationUser::setAnimation("PICKUP_AMMO_SEEKER"); break;
        case COLLTYPE_GEM_RED:      
            AnimationUser::setAnimation("PICKUP_GEM");
            color = { 511, 0, 0 };
            break;
        case COLLTYPE_GEM_GREEN:    
            AnimationUser::setAnimation("PICKUP_GEM");
            color = { 0, 511, 0 };
            break;
        case COLLTYPE_GEM_BLUE:     
            AnimationUser::setAnimation("PICKUP_GEM");
            color = { 0, 0, 511 };
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
    if (!intact) {
        // Do not apply default movement if we haven't been shot yet
        CommonActor::tickEvent();
    }
}

void Collectible::impact(double force_h, double force_v) {
    if (intact) {
        push   += force_h * (0.9 + (qrand() % 2000) / 10000.0);
        thrust += force_v * (0.9 + (qrand() % 2000) / 10000.0);
    }
    intact = false;
}

void Collectible::DrawUpdate() {
    double wave_offset = 2.4 * cos((phase * 0.3) * PI);
    pos_y += wave_offset;
    CommonActor::DrawUpdate();
    pos_y -= wave_offset;
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
            if ((qRound(pos_x + pos_y) / 32) % 2 == 1) {
                facingLeft = true;
            }
    }
}
