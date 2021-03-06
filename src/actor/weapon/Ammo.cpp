#include "Ammo.h"
#include "../../gamestate/EventMap.h"
#include "../enemy/Enemy.h"
#include "../enemy/TurtleShell.h"
#include "../collectible/Collectible.h"
#include "../solidobj/PushBox.h"
#include "../../gamestate/ActorAPI.h"

Ammo::Ammo(const ActorInstantiationDetails& initData, std::weak_ptr<Player> firedBy, bool firedLeft,
    bool firedUp, int lifeLength, bool poweredUp)
    : CommonActor(initData), poweredUp(poweredUp), strength(1), owner(firedBy), framesLeft(lifeLength),
    firedUp(firedUp) {
    canBeFrozen = false;
    isFacingLeft = firedLeft;

    // Ignore unused member warning
    (void)this->firedUp;
}

Ammo::~Ammo() {

}

void Ammo::tickEvent() {
    // Do not use the common actor movement handler here by default
    // Each ammo type needs to implement additional movement code separately
    checkCollisions();
    framesLeft--;
    if (framesLeft <= 0) {
        health = 0;
    }
}

std::weak_ptr<Player> Ammo::getOwner() {
    return owner;
}

int Ammo::getStrength() {
    return strength;
}

WeaponType Ammo::getType() const {
    return WEAPON_UNKNOWN;
}

void Ammo::checkCollisions() {
    auto collisions = api->findCollisionActors(shared_from_this());
    for (const auto& actor : collisions) {
        auto actorPtr = actor.lock();
        if (actorPtr == nullptr) {
            continue;
        }

        actorPtr->handleCollision(shared_from_this());

        // TODO: Use actor type specifying function instead when available
        if ((std::dynamic_pointer_cast<Enemy>(actorPtr) != nullptr) ||
            (std::dynamic_pointer_cast<PushBox>(actorPtr) != nullptr) ||
            (std::dynamic_pointer_cast<TurtleShell>(actorPtr) != nullptr)) {
            decreaseHealth(1);
            return;
        }
    }
    

    auto events = api->getGameEvents().lock();
    auto tiles = api->getGameTiles().lock();
    if (events != nullptr) {
        PCEvent e = events->getPositionEvent(pos);
        switch(e) {
            case PC_MODIFIER_RICOCHET:
                ricochet();
                break;
            default:
                break;
        }
    }

    auto type = getType();
    if (tiles != nullptr && tiles->checkWeaponDestructible(pos, type)) {
        if (type != WEAPON_FREEZER) {
            auto player = owner.lock();
            if (player != nullptr) {
                player->addScore(50);
            }
        }

        health = 0;
    }
}

void Ammo::ricochet() {
    speed.x = speed.x * -0.9 + (qrand() % 100 - 50.0) / 80;
    speed.y = speed.y * -0.9 + (qrand() % 100 - 50.0) / 80;
}
