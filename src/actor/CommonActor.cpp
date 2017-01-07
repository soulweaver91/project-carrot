#include "CommonActor.h"

#include <cmath>
#include "../gamestate/ActorAPI.h"
#include "../gamestate/EventMap.h"
#include "../gamestate/GameView.h"
#include "../gamestate/ResourceManager.h"
#include "../struct/PCEvent.h"
#include "../struct/Constants.h"
#include "../struct/DebugConfig.h"
#include "weapon/AmmoFreezer.h"
#include "weapon/AmmoToaster.h"

CommonActor::CommonActor(std::shared_ptr<ActorAPI> gameRoot, double x, double y, bool fromEventMap)
    : AnimationUser(gameRoot), api(gameRoot), maxHealth(1), health(1), posX(x), posY(y),
    speedX(0), speedY(0), externalForceX(0), externalForceY(0), internalForceY(0),
    canJump(false), canBeFrozen(true), isFacingLeft(false), isGravityAffected(true), isClippingAffected(true),
    isInvulnerable(false), isBlinking(false), isCollidable(true), frozenFramesLeft(0), elasticity(0.0),
    friction(api->getGravity() / 3), suspendType(SuspendType::SUSPEND_NONE), isCreatedFromEventMap(fromEventMap) {
    originTileX = static_cast<int>(x) / 32;
    originTileY = static_cast<int>(y) / 32;
    updateHitbox();
}

CommonActor::~CommonActor() {

}

void CommonActor::drawUpdate(std::shared_ptr<GameView>& view) {
    auto canvas = view->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    // Don't draw anything if we aren't in the vicinity of the view
    CoordinatePair viewPos = view->getViewCenter();
    if ((std::abs(viewPos.x - posX) > (view->getViewWidth() / 2) + 50)
     || (std::abs(viewPos.y - posY) > (view->getViewHeight() / 2) + 50)) {
        return;
    }

#ifdef CARROT_DEBUG
    if (api->getDebugConfig()->dbgShowMasked) {
        double len = sqrt(speedX * speedX + speedY * speedY);
        if (len > 0) {
            sf::RectangleShape line(sf::Vector2f(len * 4 + 5, 5));
            line.setPosition(posX, posY);
            line.setOrigin(2, 2);
            double ang = atan2(speedY, speedX);
            line.setRotation(180 + ang * 180 / 3.1415926535);
            line.setFillColor(sf::Color(255, 255, 0));
            canvas->draw(line);
        }
    }
#endif
    
    if (!((isBlinking) && ((api->getFrame() % 6) > 2))) {
        // Pick the appropriate animation depending on if we are in the midst of a transition
        auto& source = (inTransition ? transition : currentAnimation);
    
        source->setSpritePosition({ (float)posX, (float)posY }, { (isFacingLeft ? -1.0f : 1.0f), 1.0f });

        if (frozenFramesLeft > 0) {
            auto actualColor = source->getColor();
            source->setColor({ 128, 512, 512 });
            drawCurrentFrame(view);
            source->setColor(actualColor);
        } else {
            drawCurrentFrame(view);
        }
    }
}

void CommonActor::tickEvent() {
    tryStandardMovement();
    updateHitbox();

    // Make sure we stay within the level boundaries
    auto tiles = api->getGameTiles().lock();
    if (tiles != nullptr) {
        posX = std::min(std::max(posX, 0.0), tiles->getLevelWidth() * 32.0 - 1.0);
        posY = std::min(std::max(posY, 0.0), tiles->getLevelHeight() * 32.0 - 1.0);
    }
} 

void CommonActor::setToViewCenter(std::shared_ptr<GameView> view) {
    auto tiles = api->getGameTiles().lock();
    if (tiles == nullptr) {
        return;
    }

    view->centerView(
        std::max(400.0, std::min(32.0 * tiles->getLevelWidth()  - 400.0, (double)qRound(posX))),
        std::max(300.0, std::min(32.0 * tiles->getLevelHeight() - 300.0, (double)qRound(posY)))
    );
}

CoordinatePair CommonActor::getPosition() {
    return { posX, posY };
}

void CommonActor::updateHitbox() {
    updateHitbox(currentGraphicState.boundingBox.width, currentGraphicState.boundingBox.height);
}

void CommonActor::updateHitbox(const uint& w, const uint& h) {
    auto animation = currentAnimation->getAnimation();
    if (animation->hasColdspot) {
        currentHitbox = {
            posX - animation->hotspot.x + animation->coldspot.x - (w / 2),
            posY - animation->hotspot.y + animation->coldspot.y - h,
            posX - animation->hotspot.x + animation->coldspot.x + (w / 2),
            posY - animation->hotspot.y + animation->coldspot.y
        };
    } else {
        // Collision base set to the bottom of the sprite.
        // This is probably still not the correct way to do it, but at least it works for now.
        currentHitbox = {
            posX - (w / 2),
            posY - animation->hotspot.y + animation->frameDimensions.y - h,
            posX + (w / 2),
            posY - animation->hotspot.y + animation->frameDimensions.y
        };
    }
}

double CommonActor::getSpeedX() {
    return speedX;
}

double CommonActor::getSpeedY() {
    return speedY;
}

bool CommonActor::getIsCollidable() {
    return isCollidable;
}

bool CommonActor::setAnimation(AnimStateT state) {
    AnimStateT oldstate = currentAnimation->getAnimationState();
    if ((oldstate == state) || ((inTransition) && (!cancellableTransition))) {
        return true;
    }

    bool changed = AnimationUser::setAnimation(state);
    if (!changed) {
        return false;
    }

    AnimStateT newstate = state;

    switch (oldstate) {
        case AnimState::RUN:
            if ((newstate == AnimState::IDLE) || (newstate == AnimState::WALK)) {
                setTransition(AnimState::TRANSITION_RUN_TO_IDLE, true);
            }
            if (newstate == AnimState::DASH) {
                setTransition(AnimState::TRANSITION_RUN_TO_DASH, true);
            }
            break;
        case AnimState::FALL:
            if (newstate == AnimState::IDLE) {
                setTransition(AnimState::TRANSITION_IDLE_FALL_TO_IDLE, true);
            }
            break;
        case AnimState::IDLE:
            if (newstate == AnimState::JUMP) {
                setTransition(AnimState::TRANSITION_IDLE_TO_IDLE_JUMP, true);
            }
            break;
        case AnimState::SHOOT:
            if (newstate == AnimState::IDLE) {
                setTransition(AnimState::TRANSITION_IDLE_SHOOT_TO_IDLE, true);
            }
            break;
    }

    updateHitbox();
    return true;
}

void CommonActor::decreaseHealth(unsigned amount) {
    if (amount > health) {
        health = 0;
        return;
    }
    health -= amount;
}

bool CommonActor::perish() {
    // Delayed deletion routine for all actors, run after all actors' tick events
    // If health drops to 0, delete self and remove spawner from event map
    // Can be overridden, e.g. the player object instead implements death routines
    // like loading last save point here

    auto events = api->getGameEvents().lock();
    if (health == 0) {
        if (events != nullptr && isCreatedFromEventMap) {
            events->deactivate(originTileX, originTileY);
            events->storeTileEvent(originTileX, originTileY, PC_EMPTY);
        }

        api->removeActor(shared_from_this());
        return true;
    }
    return false;
}

void CommonActor::onHitFloorHook() {
    // Called from inside the position update code when the object hits floor
    // and was falling earlier. Objects should override this if they need to
    // (e.g. the Player class playing a sound).
}

void CommonActor::onHitCeilingHook() {
    // Called from inside the position update code when the object hits ceiling.
    // Objects should override this if they need to.
}

void CommonActor::onHitWallHook() {
    // Called from inside the position update code when the object hits a wall.
    // Objects should override this if they need to.
}

bool CommonActor::loadResources(const QString& classId) {
    auto loadedResources = api->loadActorTypeResources(classId);
    if (loadedResources != nullptr) {
        resources = loadedResources;
        loadAnimationSet(resources->graphics);
        return true;
    }
    
    return false;
}

void CommonActor::tryStandardMovement() {
    double gravity = (isGravityAffected ? api->getGravity() : 0);

    speedX = std::min(std::max(speedX, -16.0), 16.0);
    speedY = std::min(std::max(speedY - internalForceY - externalForceY, -16.0), 16.0);

    bool frozen = frozenFramesLeft > 0;
    double effectiveSpeedX = speedX + externalForceX;
    double effectiveSpeedY = speedY;
    if (frozen) {
        effectiveSpeedX = std::min(std::max(externalForceX, -16.0), 16.0);
        effectiveSpeedY = std::min(std::max(speedY + internalForceY, -16.0), 16.0);
    }

    auto thisPtr = shared_from_this();
    bool success = false;

    if (canJump) {
        // All ground-bound movement is handled here. In the basic case, the actor
        // moves horizontally, but it can also logically move up or down if it is
        // moving across a slope. In here, angles between about 45 degrees down
        // to 45 degrees up are attempted with some intervals to attempt to keep
        // the actor attached to the slope in question.

        // Always try values a bit over the 45 degree incline; subpixel coordinates
        // may mean the actor actually needs to move a pixel up or down even though
        // the speed wouldn't warrant that large of a change.
        // Not doing this will cause hiccups with uphill slopes in particular.
        // Beach tileset also has some spots where two properly set up adjacent
        // tiles have a 2px jump, so adapt to that.
        double maxYDiff = std::max(2.0, std::abs(effectiveSpeedX) + 2.5);
        for (double yDiff = maxYDiff; yDiff >= -maxYDiff; yDiff -= 0.1) {
            if (moveInstantly({ effectiveSpeedX, yDiff }, false, false)) {
                success = true;
                break;
            }
        }

        // Also try to move horizontally as far as possible.
        double maxDiff = std::abs(effectiveSpeedX);
        double xDiff = maxDiff;
        if (!success) {
            short sign = effectiveSpeedX > 0 ? 1 : -1;
            for (; xDiff >= -maxDiff; xDiff -= COLLISION_CHECK_STEP) {
                if (moveInstantly({ xDiff * sign, 0.0 }, false, false)) {
                    break;
                }
            }
        }

        // If no angle worked in the previous step, the actor is facing a wall.
        if (!success) {
            if (xDiff > COLLISION_CHECK_STEP || (xDiff > 0 && elasticity > 0)) {
                speedX = -(elasticity * speedX);
            }
            onHitWallHook();
        }

        // Run all floor-related hooks, such as the player's check for hurting positions.
        onHitFloorHook();
    } else {
        // Airborne movement is handled here.
        // First, attempt to move directly based on the current speed values.
        if (moveInstantly({ effectiveSpeedX, effectiveSpeedY }, false, false)) {
            if (std::abs(effectiveSpeedY) < EPSILON) {
                canJump = true;
            }
        } else if (!success) {
            // There is an obstacle so we need to make compromises.
            
            // First, attempt to move horizontally as much as possible.
            double maxDiff = std::abs(effectiveSpeedX);
            short sign = effectiveSpeedX > 0 ? 1 : -1;
            bool successX = false;
            double xDiff = maxDiff;
            for (; xDiff > EPSILON; xDiff -= COLLISION_CHECK_STEP) {
                if (moveInstantly({ xDiff * sign, 0.0 }, false, false)) {
                    successX = true;
                    break;
                }
            }

            // Then, try the same vertically.
            maxDiff = std::abs(effectiveSpeedY);
            sign = effectiveSpeedY > 0 ? 1 : -1;
            bool successY = false;
            double yDiff = maxDiff;
            for (; yDiff > EPSILON; yDiff -= COLLISION_CHECK_STEP) {
                if (moveInstantly({ 0.0, yDiff * sign }, false, false)) {
                    successY = true;
                    break;
                }
            }

            // Place us to the ground only if no horizontal movement was
            // involved (this prevents speeds resetting if the actor
            // collides with a wall from the side while in the air)
            if (yDiff < std::abs(effectiveSpeedY)) {
                if (effectiveSpeedY > 0) {
                    speedY = -(elasticity * effectiveSpeedY);
                    if (speedY > -COLLISION_CHECK_STEP) {
                        speedY = 0.0;
                        canJump = true;
                    }
                    onHitFloorHook();
                } else {
                    speedY = 0;
                    onHitCeilingHook();
                }
            }

            // If the actor didn't move all the way horizontally,
            // it hit a wall (or was already touching it)
            if (xDiff < std::abs(effectiveSpeedX)) {
                if (xDiff > COLLISION_CHECK_STEP || (xDiff > 0 && elasticity > 0)) {
                    speedX = -(elasticity * speedX);
                }
                onHitWallHook();
            }
        }
    }

    // Set the actor as airborne if there seems to be enough space below it
    if (api->isPositionEmpty(currentHitbox + CoordinatePair(0.0, api->getGravity()), effectiveSpeedY >= 0, thisPtr)) {
        speedY += gravity;
        canJump = false;
    }

    // Reduce all forces if they are present
    if (std::abs(externalForceX) > EPSILON) {
        if (externalForceX > 0) {
            externalForceX = std::max(externalForceX - friction, 0.0);
        } else {
            externalForceX = std::min(externalForceX + friction, 0.0);
        }
    }
    externalForceY = std::max(externalForceY - gravity / 3, 0.0);
    internalForceY = std::max(internalForceY - gravity / 3, 0.0);
}

template<typename... P>
bool CommonActor::playSound(const QString& id, P... params) {
    return callPlaySound(id, getPosition(), params...);
}

template<typename... P>
bool CommonActor::playNonPositionalSound(const QString& id, P... params) {
    return callPlaySound(id, false, params...);
}

template<typename T, typename... P>
bool CommonActor::callPlaySound(const QString& id, T coordOrBool, P... params) {
    auto sounds = resources->sounds.values(id);
    if (sounds.length() > 0) {
        api->playSound(sounds.at(qrand() % sounds.length()).sound, coordOrBool, params...);
        return true;
    }

    return false;
}

// Specify allowed template instantations. This is apparently required to avoid implementing the template in the header.
template bool CommonActor::playSound(const QString&);
template bool CommonActor::playSound(const QString&, float);
template bool CommonActor::playSound(const QString&, float, float);
template bool CommonActor::playSound(const QString&, float, float, float);
template bool CommonActor::playNonPositionalSound(const QString&);
template bool CommonActor::playNonPositionalSound(const QString&, float);
template bool CommonActor::playNonPositionalSound(const QString&, float, float);
template bool CommonActor::playNonPositionalSound(const QString&, float, float, float);

bool CommonActor::deactivate(int x, int y, int tileDistance) {
    auto events = api->getGameEvents().lock();

    if ((std::abs(x - originTileX) > tileDistance) || (std::abs(y - originTileY) > tileDistance)) {
        if (events != nullptr) {
            events->deactivate(originTileX, originTileY);
        }

        api->removeActor(shared_from_this());
        return true;
    }
    return false;
}

void CommonActor::handleCollision(std::shared_ptr<CommonActor> other) {
    // Objects should override this if they need to.

    if (canBeFrozen) {
        handleAmmoFrozenStateChange(other);
    }
}

bool CommonActor::moveInstantly(CoordinatePair location, bool absolute, bool force) {
    if (!absolute && std::abs(location.x) < EPSILON && std::abs(location.y) < EPSILON) {
        return true;
    }

    CoordinatePair newPos = (absolute ? location : location + CoordinatePair(posX, posY));
    auto translatedHitbox = currentHitbox + newPos - CoordinatePair(posX, posY);

    bool free = force || api->isPositionEmpty(translatedHitbox, speedY >= 0, shared_from_this());
    if (free) {
        currentHitbox = translatedHitbox;
        posX = newPos.x;
        posY = newPos.y;
    }

    return free;
}

void CommonActor::deleteFromEventMap() {
    auto events = api->getGameEvents().lock();
    if (events != nullptr) {
        events->storeTileEvent(originTileX, originTileY, PC_EMPTY);
    }
}

void CommonActor::updateGraphicState() {
    const auto& source = inTransition ? transition : currentAnimation;

    currentGraphicState = source->getGraphicState();
    currentGraphicState.scale.x = isFacingLeft ? -1.0 : 1.0;
    currentGraphicState.origin = { (float)posX, (float)posY };
}

const ActorGraphicState CommonActor::getGraphicState() {
    return currentGraphicState;
}

void CommonActor::handleAmmoFrozenStateChange(std::shared_ptr<CommonActor> ammo) {
    // TODO: Use actor type specifying function instead when available
    std::shared_ptr<AmmoFreezer> freezer = std::dynamic_pointer_cast<AmmoFreezer>(ammo);
    if (freezer != nullptr && freezer->getOwner().lock() != std::dynamic_pointer_cast<Player>(shared_from_this())) {
        frozenFramesLeft = freezer->getFrozenDuration();
    }
    if (std::dynamic_pointer_cast<AmmoToaster>(ammo) != nullptr) {
        frozenFramesLeft = 0;
    }
}

Hitbox CommonActor::getHitbox() {
    return currentHitbox;
}

void CommonActor::setInvulnerability(uint frames, bool blink) {
    isInvulnerable = true;
    isBlinking = blink;
    addTimer(frames, false, [this]() {
        isInvulnerable = false;
        isBlinking = false;
    });
}

void CommonActor::advanceActorAnimationTimers() {
    if (frozenFramesLeft == 0) {
        advanceAnimationTimers();
    } else if (frozenFramesLeft < UINT_MAX) {
        --frozenFramesLeft;
    }
}

const double CommonActor::COLLISION_CHECK_STEP = 0.5;
