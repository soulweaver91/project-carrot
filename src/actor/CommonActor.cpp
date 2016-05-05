#include "CommonActor.h"

#include "../CarrotQt5.h"
#include "../gamestate/EventMap.h"
#include "../gamestate/GameView.h"
#include "../gamestate/ResourceManager.h"
#include "../struct/PCEvent.h"

CommonActor::CommonActor(std::shared_ptr<CarrotQt5> gameRoot, double x, double y, bool fromEventMap)
    : AnimationUser(gameRoot), root(gameRoot), speedX(0), speedY(0), externalForceX(0), externalForceY(0), 
    canJump(false), isFacingLeft(false), maxHealth(1), health(1), isGravityAffected(true), internalForceY(0),
    isClippingAffected(true), elasticity(0.0), isInvulnerable(false), friction(root->gravity / 3),
    isBlinking(false), suspendType(SuspendType::SUSPEND_NONE), posX(x), posY(y), isCreatedFromEventMap(fromEventMap) {
    originTileX = static_cast<int>(x) / 32;
    originTileY = static_cast<int>(y) / 32;
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
    if (root->dbgShowMasked) {
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
    
    if (!((isBlinking) && ((root->getFrame() % 6) > 2))) {
        // Pick the appropriate animation depending on if we are in the midst of a transition
        auto& source = (inTransition ? transition : currentAnimation);
    
        source->setSpritePosition({ (float)posX, (float)posY }, { (isFacingLeft ? -1.0f : 1.0f), 1.0f });
        drawCurrentFrame(view);
    }
}

void CommonActor::processControlDownEvent(const ControlEvent& e) {
    // nothing to do in this event unless a child class
    // overrides the function
}

void CommonActor::processControlUpEvent(const ControlEvent& e) {
    // nothing to do in this event unless a child class
    // overrides the function
}

void CommonActor::processControlHeldEvent(const ControlEvent& e) {
    // nothing to do in this event unless a child class
    // overrides the function
}

void CommonActor::processAllControlHeldEvents(const QMap<Control, ControlState>& e) {
    // By default, there is no functionality here. Most actors do not process control
    // events at all. If they do, they can override this function and call the
    // processAllControlHeldEventsDefaultHandler function for easy handling of each
    // key directly in processControlHeldEvent. Keys that do one thing if they are
    // held and another if not should not be handled there but here directly, though.
}

void CommonActor::processAllControlHeldEventsDefaultHandler(const QMap<Control, ControlState>& e) {
    // By default, just go through all events.
    for (const auto& ev : e.keys()) {
        processControlHeldEvent(qMakePair(ev, e.value(ev)));
    }
}

void CommonActor::tickEvent() {
    // Sign of the speed: either -1, 0 or 1
    short sign = ((speedX + externalForceX) > 1e-6) ? 1 : (((speedX + externalForceX) < -1e-6) ? -1 : 0);
    double gravity = (isGravityAffected ? root->gravity : 0);
   
    speedX = std::min(std::max(speedX, -16.0), 16.0);
    speedY = std::min(std::max(speedY + gravity - internalForceY - externalForceY, -16.0), 16.0);

    auto thisPtr = shared_from_this();

    Hitbox currentHitbox = getHitbox();
    if (!root->isPositionEmpty(currentHitbox + CoordinatePair(speedX + externalForceX, speedY), speedY > 0, thisPtr)) {
        if (abs(speedX + externalForceX) > 1e-6) {
            // We are walking, thus having both vertical and horizontal speed
            if (root->isPositionEmpty(currentHitbox + CoordinatePair(speedX + externalForceX, 0.0), speedY > 0, thisPtr)) {
                // We could go toward the horizontal direction only
                // Chances are we're just casually strolling and gravity tries to pull us through,
                // or we are falling diagonally and hit a floor
                if (speedY > 0) {
                    // Yep, that's it; just negate the gravity effect
                    speedY = -(elasticity * speedY);
                    onHitFloorHook();
                    canJump = true;
                } else {
                    // Nope, hit a wall from below diagonally then. Let's bump back a bit
                    speedY = 1;
                    externalForceY = 0;
                    internalForceY = 0;
                    canJump = false;
                    onHitCeilingHook();
                }
            } else {
                // Nope, there's also some obstacle horizontally
                // Let's figure out if we are going against an upward slope
                if (root->isPositionEmpty(currentHitbox + CoordinatePair(speedX + externalForceX, -abs(speedX + externalForceX) - 5), false, thisPtr)) {
                    // Yes, we indeed are
                    speedY = -(elasticity * speedY);
                    canJump = true;
                    posY -= abs(speedX + externalForceX) + 1;
                    /*while (root->game_tiles->isTileEmpty(CarrotQt5::calcHitbox(getHitbox(),speedX,speedY-abs(speedX)-2))) {
                        posY += 0.5;
                    }
                    posY -= 1;*/
                    
                    // TODO: position us vertically to a suitable position so that we won't raise up to the air needlessly
                    // (applicable if the slope isn't exactly at 45 degrees)
                } else {
                    // Nope. Cannot move horizontally at all. Can we just go vertically then?
                    speedX = -(elasticity * speedX);
                    externalForceX *= -1;
                    if (root->isPositionEmpty(currentHitbox + CoordinatePair(0.0, speedY), speedY > 0, thisPtr)) {
                        // Yeah
                        canJump = false;
                        onHitWallHook();
                    } else {
                        // Nope
                        if (speedY > 0) {
                            canJump = true;
                        }

                        speedY = -(elasticity * speedY);
                        externalForceY = 0;
                        internalForceY = 0;
                        // TODO: fix a problem with hurt getting player stuck in a wall here
                        onHitWallHook();
                        onHitFloorHook();
                    }
                }
            }
        } else {
            // We are going directly vertically
            if (speedY > 0) {
                // We are falling, or we are on solid ground and gravity tries to push us through the floor
                if (root->isPositionEmpty(currentHitbox, true, shared_from_this())) {
                    // Let's just nullify that effect
                    speedY = -(elasticity * speedY);
                    while (root->isPositionEmpty(getHitbox().add(speedX, speedY), true, thisPtr)) {
                        posY += 0.5;
                    }
                    posY -= 0.5;
                    
                    onHitFloorHook();
                    canJump = true;
                } else {
                    // Nope, nothing else is going on
                    speedY = -(elasticity * speedY);
                    canJump = true;
                }
            } else {
                // We are jumping
                if (!root->isPositionEmpty(currentHitbox + CoordinatePair(0.0, speedY), false, thisPtr)) {
                    speedY = -(elasticity * speedY);
                    externalForceY = 0;
                    internalForceY = 0;
                    onHitCeilingHook();
                } else {
                    // we can go vertically anyway
                    //speedY = -(elasticity * speedY);
                }
            }
        }
    } else {
        if (canJump) {
            // Check if we are running on a downhill slope. If so, keep us attached to said slope instead of flying off.
            if (!root->isPositionEmpty(
                currentHitbox + CoordinatePair(speedX + externalForceX, speedY + abs(speedX + externalForceX) + 5),
                false, thisPtr)
            ) {
                while (root->isPositionEmpty(
                    getHitbox().add(speedX + externalForceX, speedY + abs(speedX + externalForceX)),
                    false, thisPtr)
                ) {
                    posY += 0.1;
                }
                posY -= 0.1;
            } else {
                // That wasn't the case so forget about that
                canJump = false;
            }
        }
    }

    if (std::abs(externalForceX) > 1e-6) {
        // Reduce remaining push
        if (externalForceX > 0) {
            externalForceX = std::max(externalForceX - friction, 0.0);
        } else {
            externalForceX = std::min(externalForceX + friction, 0.0);
        }
    }
    externalForceY = std::max(externalForceY - gravity / 3, 0.0);
    internalForceY = std::max(internalForceY - gravity / 3, 0.0);
    //speedX = sign * std::max((sign * speedX) - friction, 0.0);

    posX += speedX + externalForceX;
    posY += speedY;

    // determine current animation last bits from speeds
    // it's okay to call setAnimation on every tick because it doesn't do
    // anything if the animation is the same as it was earlier

    // only certain ones don't need to be preserved from earlier state, others should be set as expected
    int composite = currentAnimation->getAnimationState() & 0xFFFFFFE0;
    if (abs(speedX) > 3) {
        // shift-running, speed is more than 3px/frame
        composite += 3;
    } else if (abs(speedX) > 1) {
        // running, speed is between 1px and 3px/frame
        composite += 2;
    } else if (abs(speedX) > 1e-6) {
        // walking, speed is less than 1px/frame (mostly a transition zone)
        composite += 1;
    }
    
    if (suspendType != SuspendType::SUSPEND_NONE) {
        composite += 12;
    } else {
        if (canJump) {
            // grounded, no vertical speed
        } else if (speedY > 1e-6) {
            // falling, ver. speed is positive
            composite += 8;
        } else if (speedY < -1e-6) {
            // jumping, ver. speed is negative
            composite += 4;
        }
    }
    
    AnimStateT newState = AnimStateT(composite);
    setAnimation(newState);

    // Make sure we stay within the level boundaries
    auto tiles = root->getGameTiles().lock();
    if (tiles != nullptr) {
        posX = std::min(std::max(posX, 0.0), tiles->getLevelWidth() * 32.0);
        posY = std::min(std::max(posY, 0.0), tiles->getLevelHeight() * 32.0 + 31.0);
    }
} 

void CommonActor::setToViewCenter(std::shared_ptr<GameView> view) {
    auto tiles = root->getGameTiles().lock();
    if (tiles == nullptr) {
        return;
    }

    view->centerView(
        std::max(400.0, std::min(32.0 * (tiles->getLevelWidth() + 1)  - 400.0, (double)qRound(posX))),
        std::max(300.0, std::min(32.0 * (tiles->getLevelHeight() + 1) - 300.0, (double)qRound(posY)))
    );
}

CoordinatePair CommonActor::getPosition() {
    return { posX, posY };
}

Hitbox CommonActor::getHitbox() {
    auto animation = currentAnimation->getAnimation();
    return getHitbox(animation->frameDimensions.x, animation->frameDimensions.y);
}

Hitbox CommonActor::getHitbox(const uint& w, const uint& h) {
    auto animation = currentAnimation->getAnimation();
    if (animation->hasColdspot) {
        return {
            posX - animation->hotspot.x + animation->coldspot.x - (w / 2),
            posY - animation->hotspot.y + animation->coldspot.y - h,
            posX - animation->hotspot.x + animation->coldspot.x + (w / 2),
            posY - animation->hotspot.y + animation->coldspot.y
        };
    } else {
        // Collision base set to the bottom of the sprite.
        // This is probably still not the correct way to do it, but at least it works for now.
        return {
            posX - (w / 2),
            posY - animation->hotspot.y + animation->frameDimensions.y - h,
            posX + (w / 2),
            posY - animation->hotspot.y + animation->frameDimensions.y
        };
    }
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

    auto events = root->getGameEvents().lock();
    if (health == 0) {
        if (events != nullptr && isCreatedFromEventMap) {
            events->deactivate(originTileX, originTileY);
            events->storeTileEvent(originTileX, originTileY, PC_EMPTY);
        }

        root->removeActor(shared_from_this());
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
    auto loadedResources = root->loadActorTypeResources(classId);
    if (loadedResources != nullptr) {
        resources = loadedResources;
        loadAnimationSet(resources->graphics);
        return true;
    }
    
    return false;
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
    auto soundSystem = root->getSoundSystem().lock();
    if (soundSystem == nullptr) {
        return false;
    }

    auto sounds = resources->sounds.values(id);
    if (sounds.length() > 0) {
        soundSystem->playSFX(sounds.at(qrand() % sounds.length()).sound, coordOrBool, params...);
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
    auto events = root->getGameEvents().lock();

    if ((std::abs(x - originTileX) > tileDistance) || (std::abs(y - originTileY) > tileDistance)) {
        if (events != nullptr) {
            events->deactivate(originTileX, originTileY);
        }

        root->removeActor(shared_from_this());
        return true;
    }
    return false;
}

void CommonActor::moveInstantly(CoordinatePair location) {
    posX = location.x;
    posY = location.y;
}

void CommonActor::deleteFromEventMap() {
    auto events = root->getGameEvents().lock();
    if (events != nullptr) {
        events->storeTileEvent(originTileX, originTileY, PC_EMPTY);
    }
}

void CommonActor::updateGraphicState() {
    const auto& source = inTransition ? transition : currentAnimation;
    const auto& animation = source->getAnimation();

    currentGraphicState = source->getGraphicState();
    currentGraphicState.scale.x = isFacingLeft ? -1.0 : 1.0;
    currentGraphicState.origin = { (float)posX, (float)posY };
}

const ActorGraphicState CommonActor::getGraphicState() {
    return currentGraphicState;
}

void CommonActor::setInvulnerability(uint frames, bool blink) {
    isInvulnerable = true;
    isBlinking = blink;
    addTimer(frames, false, [this]() {
        isInvulnerable = false;
        isBlinking = false;
    });
}
