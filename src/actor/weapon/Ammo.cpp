#include "Ammo.h"
#include "../../gamestate/EventMap.h"
#include "../enemy/Enemy.h"
#include "../collectible/Collectible.h"
#include "../PushBox.h"

Ammo::Ammo(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> firedBy, double x, double y, bool firedLeft,
    bool firedUp, int lifeLength, bool powered)
    : CommonActor(root, x, y, false), owner(firedBy), framesLeft(lifeLength), poweredUp(powered) {
    isFacingLeft = firedLeft;
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

void Ammo::checkCollisions() {
    auto collisions = root->findCollisionActors(getHitbox(), shared_from_this());
    for (const auto& actor : collisions) {
        auto actorPtr = actor.lock();
        if (actorPtr == nullptr) {
            continue;
        }

        // Different things happen with different actor types
        auto enemy = std::dynamic_pointer_cast<Enemy>(actorPtr);
        if (enemy != nullptr) {
            enemy->decreaseHealth(1);
            decreaseHealth(1);
            return;
        }

        auto coll = std::dynamic_pointer_cast<Collectible>(actorPtr);
        if (coll != nullptr) {
            coll->impact(speedX / 5.0, -speedY / 10.0);
        }

        auto box = std::dynamic_pointer_cast<PushBox>(actorPtr);
        if (box != nullptr) {
            decreaseHealth(1);
        }

    }
    

    auto events = root->getGameEvents().lock();
    auto tiles = root->getGameTiles().lock();
    if (events != nullptr) {
        PCEvent e = events->getPositionEvent(posX, posY);
        switch(e) {
            case PC_MODIFIER_RICOCHET:
                ricochet();
        }
    }

    if (tiles != nullptr && tiles->checkWeaponDestructible(posX, posY)) {
        health = 0;
    }
}

void Ammo::ricochet() {
    speedY = speedY * -0.9 + (qrand() % 100 - 50.0) / 80;
    speedX = speedX * -0.9 + (qrand() % 100 - 50.0) / 80;
}
