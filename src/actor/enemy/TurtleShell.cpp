#include "TurtleShell.h"

#include <cmath>
#include "Enemy.h"
#include "../collectible/Collectible.h"
#include "../weapon/Ammo.h"
#include "../weapon/AmmoFreezer.h"
#include "../weapon/AmmoToaster.h"
#include "../../gamestate/ActorAPI.h"
#include "../../gamestate/TileMap.h"
#include "../../struct/Constants.h"


TurtleShell::TurtleShell(const ActorInstantiationDetails& initData, sf::Vector2f initSpeed) : CommonActor(initData) {
    loadResources("Enemy/TurtleShell");
    setAnimation(AnimState::IDLE);

    speed = initSpeed;
    friction = api->getGravity() / 100;
    elasticity = 0.5;
    // TODO: test the actual number
    maxHealth = health = 8;

    playSound("ENEMY_TURTLE_SHELL_FLY");
}

TurtleShell::~TurtleShell() {
}

void TurtleShell::tickEvent() {
    speed.x = std::max(std::abs(speed.x) - friction, 0.0) * (speed.x > EPSILON ? 1 : -1);

    double posYBefore = pos.y;
    CommonActor::tickEvent();
    if (posYBefore - pos.y > 0.5 && std::abs(speed.y) < 1) {
        speed.x = std::max(std::abs(speed.x) - 10 * friction, 0.0) * (speed.x > EPSILON ? 1 : -1);
    }

    auto collisions = api->findCollisionActors(shared_from_this());
    for (auto collider : collisions) {
        if (collider.expired()) {
            continue;
        }

        auto colliderPtr = collider.lock();

        {
            auto specializedPtr = std::dynamic_pointer_cast<Ammo>(colliderPtr);
            if (specializedPtr != nullptr) {
                playSound("ENEMY_TURTLE_SHELL_FLY");
                continue;
            }
        }

        {
            auto specializedPtr = std::dynamic_pointer_cast<Enemy>(colliderPtr);
            if (specializedPtr != nullptr && std::abs(speed.x) > 0.5) {
                specializedPtr->decreaseHealth(1);
                speed.x = std::max(std::abs(speed.x), 2.0f) * (speed.x > 0 ? -1 : 1);
                continue;
            }
        }

        {
            auto specializedPtr = std::dynamic_pointer_cast<Collectible>(colliderPtr);
            if (specializedPtr != nullptr) {
                specializedPtr->handleCollision(shared_from_this());
                continue;
            }
        }

        {
            auto specializedPtr = std::dynamic_pointer_cast<TurtleShell>(colliderPtr);
            if (specializedPtr != nullptr) {
                if (speed.y - specializedPtr->speed.y > 1 && speed.y > 0) {
                    specializedPtr->decreaseHealth(10);
                    continue;
                }

                if (std::abs(speed.x) > std::abs(specializedPtr->speed.x)) {
                    // Handle this only in the faster of the two.
                    pos.x = specializedPtr->pos.x + (speed.x > 0 ? -1 : 1) * (currentGraphicState.dimensions.x + 1);
                    double totalSpeed = std::abs(speed.x) + std::abs(specializedPtr->speed.x);

                    specializedPtr->speed.x = totalSpeed / 2 * (speed.x > 0 ? 1 : -1);
                    speed.x = totalSpeed / 2 * (speed.x > 0 ? -1 : 1);

                    specializedPtr->decreaseHealth(1);
                    playSound("ENEMY_TURTLE_SHELL_IMPACT_SHELL");
                }
                continue;
            }
        }
    }

    auto tiles = api->getGameTiles().lock();
    if (tiles != nullptr) {
        tiles->checkSpecialDestructible(currentHitbox);
        tiles->checkCollapseDestructible(currentHitbox);
        // TODO a shell ain't a blaster
        tiles->checkWeaponDestructible(pos, WEAPON_BLASTER);
    }
}

void TurtleShell::updateHitbox() {
    CommonActor::updateHitbox(24u, 16u);
}

void TurtleShell::handleCollision(std::shared_ptr<CommonActor> other) {
    CommonActor::handleCollision(other);

    // TODO: Use actor type specifying function instead when available
    if (std::dynamic_pointer_cast<Ammo>(other) != nullptr &&
        std::dynamic_pointer_cast<AmmoFreezer>(other) == nullptr) {
        if (std::dynamic_pointer_cast<AmmoToaster>(other) != nullptr) {
            health -= maxHealth;
            return;
        }

        double otherSpeed = other->getSpeed().x;
        speed.x = std::max(4.0, std::abs(otherSpeed)) * (otherSpeed < 0 ? -1 : 1) / 2;
    }
}

void TurtleShell::onHitFloorHook() {
    if (std::abs(speed.y) > 1) {
        playSound("ENEMY_TURTLE_SHELL_IMPACT_GROUND");
    }
}

