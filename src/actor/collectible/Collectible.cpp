#include "Collectible.h"

#include <memory>
#include <cmath>
#include "../Player.h"
#include "../enemy/TurtleShell.h"
#include "../../struct/Constants.h"
#include "../weapon/Ammo.h"

Collectible::Collectible(const ActorInstantiationDetails& initData, bool fromEventMap)
    : CommonActor(initData, fromEventMap), untouched(true), scoreValue(100) {
    canBeFrozen = false;
    phase = ((posX / 100.0) + (posY / 100.0));
    elasticity = 0.6;
    loadResources("Object/Collectible");
}

Collectible::~Collectible() {

}

void Collectible::tickEvent() {
    phase += 0.1;

    if (!untouched) {
        // Do not apply default movement if we haven't been shot yet
        CommonActor::tickEvent();
    }
}

void Collectible::drawUpdate(std::shared_ptr<GameView>& view) {
    double waveOffset = 2.4 * std::cos((phase * 0.3) * PI);

    // The position of the actor is altered for the draw event.
    // We want to keep the actual position of the actor constant, though.
    posY += waveOffset;
    CommonActor::drawUpdate(view);
    posY -= waveOffset;
}

void Collectible::collect(std::shared_ptr<Player> player) {
    player->addScore(scoreValue);
}

void Collectible::handleCollision(std::shared_ptr<CommonActor> other) {
    // TODO: Use actor type specifying function instead when available
    bool impactable;
    {
        auto ptr = std::dynamic_pointer_cast<Ammo>(other);
        if (ptr != nullptr) {
            impactable = true;
        }
    }
    {
        auto ptr = std::dynamic_pointer_cast<TurtleShell>(other);
        if (ptr != nullptr) {
            impactable = true;
        }
    }

    if (impactable) {
        if (untouched) {
            externalForceX += other->getSpeedX() / 5.0 * (0.9 + (qrand() % 2000) / 10000.0);
            externalForceY += -other->getSpeedY() / 10.0 * (0.9 + (qrand() % 2000) / 10000.0);
        }
        untouched = false;
    }
}

void Collectible::setFacingDirection() {
    if ((qRound(posX + posY) / 32) % 2 == 0) {
        isFacingLeft = true;
    }
}
