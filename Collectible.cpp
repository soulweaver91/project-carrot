#include <memory>
#include "CommonActor.h"
#include "CarrotQt5.h"
#include "Collectible.h"

#define PI 3.1415926535

Collectible::Collectible(std::shared_ptr<CarrotQt5> root, enum CollectibleType type, double x, double y, bool fromEventMap)
    : CommonActor(root, x, y, fromEventMap), type(type), intact(true) {
    phase = ((x / 100.0) + (y / 100.0));
    elasticity = 0.6;

    // temporary code
    switch(type) {
        case COLLTYPE_FAST_FIRE:    addAnimation(AnimState::IDLE, "pickup/fast_fire_jazz.png",10,1,15,23,10,5,12); break;
        case COLLTYPE_AMMO_TOASTER: addAnimation(AnimState::IDLE, "pickup/ammo_toaster.png",  10,1,16,14,10,5,7); break;
        case COLLTYPE_AMMO_BOUNCER: addAnimation(AnimState::IDLE, "pickup/ammo_bouncer.png",  10,1,16,13,10,5,7); break;
        case COLLTYPE_AMMO_SEEKER:  addAnimation(AnimState::IDLE, "pickup/ammo_seeker.png",   10,1,19,20,10,7,10); break;
        case COLLTYPE_GEM_RED:      addAnimation(AnimState::IDLE, "pickup/gem.png",           8,1,25,26,10,10,13); if ((qRound(x+y) / 32) % 2 == 1) { facingLeft = true; } break;
        case COLLTYPE_GEM_GREEN:    addAnimation(AnimState::IDLE, "pickup/gem.png",           8,1,25,26,10,10,13); if ((qRound(x+y) / 32) % 2 == 1) { facingLeft = true; } break;
        case COLLTYPE_GEM_BLUE:     addAnimation(AnimState::IDLE, "pickup/gem.png",           8,1,25,26,10,10,13); if ((qRound(x+y) / 32) % 2 == 1) { facingLeft = true; } break;
        case COLLTYPE_COIN_GOLD:    addAnimation(AnimState::IDLE, "pickup/coin_gold.png",     20,1,24,23,10,13,12); if ((qRound(x+y) / 32) % 2 == 1) { facingLeft = true; } break;
        case COLLTYPE_COIN_SILVER:  addAnimation(AnimState::IDLE, "pickup/coin_silver.png",   20,1,25,23,10,13,12); if ((qRound(x+y) / 32) % 2 == 1) { facingLeft = true; } break;
    }
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
