#include "Player.h"

#include <cmath>
#include "../gamestate/ActorAPI.h"
#include "../gamestate/EventMap.h"
#include "../gamestate/GameView.h"
#include "../gamestate/TileMap.h"
#include "solidobj/SolidObject.h"
#include "solidobj/TriggerCrate.h"
#include "enemy/Enemy.h"
#include "enemy/TurtleShell.h"
#include "collectible/Collectible.h"
#include "SavePoint.h"
#include "Spring.h"
#include "BonusWarp.h"
#include "solidobj/PowerUpMonitor.h"
#include "solidobj/GenericContainer.h"
#include "weapon/AmmoBlaster.h"
#include "weapon/AmmoBouncer.h"
#include "weapon/AmmoToaster.h"
#include "weapon/AmmoFreezer.h"

Player::Player(const ActorInstantiationDetails& initData) : InteractiveActor(initData),
    RadialLightSource(50.0, 100.0),
    character(CHAR_JAZZ), lives(3), fastfires(0), score(0), foodCounter(0), currentWeapon(WEAPON_BLASTER),
    weaponCooldown(0), currentSpecialMove(SPECIAL_MOVE_NONE), isAttachedToPole(false), isActivelyPushing(false),
    cameraShiftFramesCount(0), copterFramesLeft(0), levelExiting(false), toasterAmmoSubticks(10), isSugarRush(false) {
    loadResources("Interactive/PlayerJazz");

    maxHealth = 5;
    health = 5;
    std::fill_n(ammo, WEAPONCOUNT, 0);
    std::fill_n(isWeaponPoweredUp, WEAPONCOUNT, false);
    std::fill_n(collectedCoins, 2, 0);
    std::fill_n(collectedGems, 4, 0);

    setAnimation(AnimState::FALL);

    controls = {
        Qt::Key::Key_Left,
        Qt::Key::Key_Right,
        Qt::Key::Key_Up,
        Qt::Key::Key_Down,
#ifdef Q_OS_MAC
        // using control (Key_Meta) or command (Key_Control) causes all sorts of funny problems
        Qt::Key::Key_Alt,
#else
        Qt::Key::Key_Control,
#endif
        Qt::Key::Key_Space,
        Qt::Key::Key_Shift,
        Qt::Key::Key_Return
    };

    // Get a brief invincibility at the start of the level
    setInvulnerability(210u, true);
}

Player::~Player() {

}

void Player::processControlDownEvent(const ControlEvent& e) {
    const Control& control = e.first;
    const AnimStateT currentState = currentAnimation->getAnimationState();

    if (control == controls.leftButton || control == controls.rightButton) {
        if (controllable) {
            setAnimation(currentState & ~(AnimState::LOOKUP | AnimState::CROUCH));
        }
        return;
    }

    if (control == controls.upButton) {
        if (controllable && canJump && (std::abs(speed.x) < EPSILON)) {
            setAnimation(AnimState::LOOKUP);
        }
        return;
    }

    if (control == controls.fireButton) {
        // only TNT needs to be handled here
        if (canJump) {
        }
        return;
    }

    if (control == controls.downButton) {
        if (controllable) {
            if (canJump) {
                if (std::abs(speed.x) < EPSILON) {
                    setAnimation(AnimState::CROUCH);
                }
            } else {
                if (suspendType != SuspendType::SUSPEND_NONE) {
                    moveInstantly({ 0, 10 }, false, true);
                    suspendType = SuspendType::SUSPEND_NONE;
                } else {
                    controllable = false;
                    speed = { 0, 0 };
                    internalForce.y = 0;
                    externalForce.y = 0;
                    isGravityAffected = false;
                    currentSpecialMove = SPECIAL_MOVE_BUTTSTOMP;
                    setAnimation(AnimState::BUTTSTOMP);
                    setPlayerTransition(AnimState::TRANSITION_BUTTSTOMP_START, true, false, SPECIAL_MOVE_BUTTSTOMP, [this]() {
                        speed.y = 9;
                        setAnimation(AnimState::BUTTSTOMP);
                        playSound("PLAYER_BUTTSTOMP", 1.0f, 0.0f, 0.8f);
                    });
                }
            }
        }
        return;
    }

    if (control == Qt::Key_1) {
        selectWeapon(WEAPON_BLASTER);
        return;
    }

    if (control > Qt::Key_1 && control < Qt::Key_9 + 1) {
        playSound("PLAYER_SWITCH_AMMO");
        // Key_2 is 50 and the rest come sequentially so no need for a separate
        // case branch for each key this way
        // Change this if the key enum codes change in Qt for any reason
        selectWeapon(static_cast<WeaponType>(control.keyboardKey - 49));
        return;
    }

    if (control == controls.weaponChangeButton) {
        // Select next available weapon in numerical order
        int new_type = (currentWeapon + 1) % WEAPONCOUNT;
        while (!selectWeapon(static_cast<WeaponType>(new_type))) {
            new_type = (new_type + 1) % WEAPONCOUNT;
        }
        return;
    }

    if (control == controls.jumpButton) {
        switch (character) {
            case CHAR_JAZZ:
                if ((currentState & AnimState::CROUCH) > 0) {
                    controllable = false;
                    setAnimation(AnimState::UPPERCUT);
                    setPlayerTransition(AnimState::TRANSITION_UPPERCUT_A, true, true, SPECIAL_MOVE_UPPERCUT, [this]() {
                        externalForce.y = 1.5;
                        speed.y = -2;
                        canJump = false;
                        setPlayerTransition(AnimState::TRANSITION_UPPERCUT_B, true, true, SPECIAL_MOVE_UPPERCUT);
                    });
                } else {
                    if (speed.y > 0 && !canJump) {
                        isGravityAffected = false;
                        speed.y = 1.5;
                        if ((currentState & AnimState::COPTER) == 0) {
                            setAnimation(AnimState::COPTER);
                        }
                        copterFramesLeft = 50;
                    }
                }
                break;
            case CHAR_SPAZ:
            case CHAR_LORI:
                // sidekick
                break;
            default:
                break;
        }
        return;
    }
}

void Player::processControlUpEvent(const ControlEvent& e) {
    const Control& control = e.first;
    const AnimStateT currentState = currentAnimation->getAnimationState();

    if (controllable) {
        if (control == controls.upButton) {
                setAnimation(currentState & ~AnimState::LOOKUP);
                return;
        }

        if (control == controls.jumpButton) {
                setAnimation(currentState & ~AnimState::SHOOT);
                weaponCooldown = 0;
                return;
        }

        if (control == controls.downButton) {
                setAnimation(currentState & ~AnimState::CROUCH);
                return;
        }
    }

    if (control == controls.fireButton) {
        weaponCooldown = std::min(2u, weaponCooldown);
    }
}

void Player::processAllControlHeldEvents(const QMap<Control, ControlState>& e) {
    const AnimStateT currentState = currentAnimation->getAnimationState();

    if (controllable && (e.contains(controls.leftButton) || e.contains(controls.rightButton))) {
        isFacingLeft = !e.contains(controls.rightButton);
        isActivelyPushing = true;

        if (suspendType == SuspendType::SUSPEND_NONE && (e.contains(controls.dashButton))) {
            speed.x = std::max(std::min(speed.x + ACCELERATION * (isFacingLeft ? -1 : 1), MAX_DASHING_SPEED), -MAX_DASHING_SPEED);
        } else if (suspendType != SuspendType::SUSPEND_HOOK) {
            speed.x = std::max(std::min(speed.x + ACCELERATION * (isFacingLeft ? -1 : 1), MAX_RUNNING_SPEED), -MAX_RUNNING_SPEED);
        }

    } else {
        speed.x = std::max((std::abs(speed.x) - DECELERATION), 0.0) * (speed.x < -EPSILON ? -1 : 1);
        isActivelyPushing = false;
    }

    if (!controllable) {
        return;
    }

    if (e.contains(controls.jumpButton)) {
        if (suspendType != SuspendType::SUSPEND_NONE) {
            moveInstantly({ 0, -5 }, false, true);
            canJump = true;
        }
        if (canJump && currentSpecialMove == SPECIAL_MOVE_NONE && !e.contains(controls.downButton)) {
            internalForce.y = 1.2;
            speed.y = -3 - std::max(0.0, (std::abs(speed.x) - 4.0) * 0.3);
            canJump = false;
            setAnimation(currentState & (~AnimState::LOOKUP & ~AnimState::CROUCH));
            playSound("COMMON_JUMP");
            carryingObject.reset();
        }
    } else {
        if (internalForce.y > 0) {
            internalForce.y = 0;
        }
    }

    processAllControlHeldEventsDefaultHandler(e);
}

void Player::processControlHeldEvent(const ControlEvent& e) {
    const AnimStateT currentState = currentAnimation->getAnimationState();

    if (e.first == controls.fireButton) {
        setAnimation(currentState | AnimState::SHOOT);
        if (weaponCooldown == 0) {
            bool poweredUp = isWeaponPoweredUp[(uint)currentWeapon];
            switch (currentWeapon) {
                case WEAPON_BLASTER:
                {
                    auto newAmmo = fireWeapon<AmmoBlaster>(poweredUp);
                    weaponCooldown = 40 - std::min(40u, 3 * fastfires);
                    playSound("WEAPON_BLASTER_JAZZ");
                    break;
                }
                case WEAPON_BOUNCER:
                {
                    auto newAmmo = fireWeapon<AmmoBouncer>(poweredUp);
                    weaponCooldown = 25;
                    break;
                }
                case WEAPON_FREEZER:
                {
                    auto newAmmo = fireWeapon<AmmoFreezer>(poweredUp);
                    weaponCooldown = 25;
                    break;
                }
                case WEAPON_TOASTER:
                {
                    auto newAmmo = fireWeapon<AmmoToaster>(poweredUp);
                    weaponCooldown = 3;
                    break;
                }
                case WEAPON_TNT:
                    // do nothing, TNT can only be placed by keypress, not by holding space down
                    break;
                case WEAPON_SEEKER:
                case WEAPON_RF:
                case WEAPON_PEPPER:
                case WEAPON_ELECTRO:
                default:
                    break;
            }
            if (currentWeapon != WEAPON_BLASTER) {
                if (currentWeapon == WEAPON_TOASTER) {
                    --toasterAmmoSubticks;
                    if (toasterAmmoSubticks == 0) {
                        ammo[currentWeapon] -= 1;
                        toasterAmmoSubticks = 10;
                    }
                } else {
                    ammo[currentWeapon] -= 1;
                }

                osd->setAmmo(ammo[currentWeapon]);
                if (ammo[currentWeapon] == 0) {
                    isWeaponPoweredUp[(uint)currentWeapon] = false;
                    int newType = (currentWeapon + 1) % WEAPONCOUNT;
                    // Iterate through weapons to pick the next usable when running out of ammo
                    while (!selectWeapon(static_cast<WeaponType>(newType))) {
                        newType = (newType + 1) % WEAPONCOUNT;
                    }
                }
            }
        }
    }
}

void Player::tickEvent() {
    verifyOSDInitialized();
    followCarryingPlatform();

    double lastX = pos.x;
    CommonActor::tickEvent();
    updateSpeedBasedAnimation(lastX);
    updateCameraPosition();

    pushSolidObjects();
    checkSuspendedStatus();
    checkDestructibleTiles();
    checkEndOfSpecialMoves();

    handleAreaEvents();
    handleActorCollisions();

    // Reduce weapon cooldown by one frame each frame
    if (weaponCooldown > 0) {
        weaponCooldown--;
    }
}

unsigned Player::getHealth() {
    return health;
}

void Player::drawUIOverlay() {
    osd->drawOSD(assignedView);
}

bool Player::selectWeapon(enum WeaponType new_type) {
    // Nothing to do if already selected the chosen type
    if (currentWeapon == new_type) {
        return true;
    }

    // If not enough ammo, don't let change types (weapon 1 always has infinite ammo)
    if ((ammo[new_type] == 0) && (new_type != WEAPON_BLASTER)) {
        return false;
    }

    currentWeapon = new_type;
    osd->setWeaponType(new_type, isWeaponPoweredUp[new_type]);
    osd->setAmmo(ammo[currentWeapon]);

    return true;
}

#ifdef CARROT_DEBUG
void Player::debugHealth() {
    health = 5;
}

void Player::debugAmmo() {
    std::fill_n(ammo, WEAPONCOUNT, 999);
}
#endif

void Player::addAmmo(enum WeaponType type, unsigned amount) {
    if (type > (WEAPONCOUNT - 1)) { return; }
    playSound("PLAYER_PICKUP_AMMO");

    if (ammo[type] == 0) {
        // Switch to the newly obtained weapon
        ammo[type] += amount;
        selectWeapon(type);
    } else {
        ammo[type] += amount;
    }

    if (type == currentWeapon) {
        osd->setAmmo(ammo[type]);
    }
}

void Player::addGems(GemType type, unsigned amount) {
    if ((uint)type >= 4) {
        return;
    }

    collectedGems[(uint)type] += amount;
    setupOSD((OSDMessageType)((uint)OSD_GEM_RED + (uint)type));
}

void Player::addCoins(CoinType type, unsigned amount) {
    if ((uint)type >= 2) {
        return;
    }

    playSound("PLAYER_PICKUP_COIN");
    collectedCoins[(uint)type] += amount;
    setupOSD((OSDMessageType)((uint)OSD_COIN_SILVER + (uint)type));
}

void Player::addFastFire(unsigned amount) {
    if (fastfires < 10) {
        osd->setMessage(OSD_CUSTOM_TEXT, "fastfire");
    }
    fastfires = std::min(fastfires + amount, 10u);
    playSound("PLAYER_PICKUP_AMMO");
}

void Player::addHealth(unsigned amount) {
    if (amount != UINT_MAX) {
        health = std::min(health + amount, maxHealth);
        playSound("PLAYER_CONSUME_FOOD");
    } else {
        health = maxHealth;
        playSound("PLAYER_JAZZ_CONSUME_MAX_CARROT");
        setInvulnerability(210u, false);
    }
    osd->setHealth(health);
}

void Player::addLives(unsigned amount) {
    lives += amount;
    osd->setLives(lives);
    playSound("PLAYER_PICKUP_ONEUP");
}

void Player::setPowerUp(WeaponType type) {
    if (type > (WEAPONCOUNT - 1)) { return; }
    uint typeIdx = (uint)type;

    if (currentWeapon == type) {
        // The OSD animation has to be refreshed if the same weapon was already selected.
        osd->setWeaponType(type, true);
    }

    if (!isWeaponPoweredUp[typeIdx]) {
        isWeaponPoweredUp[typeIdx] = true;

        if (ammo[typeIdx] == 0) {
            ammo[typeIdx] = 20;
        }

        selectWeapon(type);
        osd->setMessage(OSD_CUSTOM_TEXT, "power up");
    } else {
        ammo[typeIdx] += 20;
    }

    if (currentWeapon == type) {
        osd->setAmmo(ammo[typeIdx]);
    }
}

void Player::consumeFood(const bool& isDrinkable) {
    foodCounter += 1;
    if (foodCounter >= SUGAR_RUSH_THRESHOLD) {
        foodCounter = foodCounter % SUGAR_RUSH_THRESHOLD;
        if (!isSugarRush) {
            api->pauseMusic();
            playNonPositionalSound("PLAYER_SUGAR_RUSH");

            isSugarRush = true;
            osd->setSugarRushActive();
            addTimer(21.548 * 70.0, false, [this]() {
                isSugarRush = false;
                api->resumeMusic();
            });
        }
    }

    if (isDrinkable) {
        playSound("PLAYER_CONSUME_DRINK");
    } else {
        playSound("PLAYER_CONSUME_FOOD");
    }
}

bool Player::perish() {
    // handle death here
    if (health == 0 && transition->getAnimationState() != AnimState::TRANSITION_DEATH) {
        cancellableTransition = false;
        setPlayerTransition(AnimState::TRANSITION_DEATH, false, true, SPECIAL_MOVE_NONE, [this]() {
            if (lives > 0) {
                lives--;

                // Return us to the last save point
                api->loadSavePoint();

                // Reset health and remove one life
                health = maxHealth;
                osd->setLives(lives);
                osd->setHealth(maxHealth);

                // Negate all possible movement effects etc.
                transition->clearCallback();
                inTransition = false;
                canJump = false;
                externalForce = { 0, 0 };
                internalForce = { 0, 0 };
                speed = { 0, 0 };
                controllable = true;

                // remove fast fires
                fastfires = 0;
            } else {
                api->handleGameOver();
            }
        });
    }
    return false;
}

QSet<WeaponType> Player::getAvailableWeaponTypes() {
    QSet<WeaponType> types;
    for (int i = 0; i < WEAPONCOUNT; ++i) {
        if (ammo[i] > 0) {
            types << (WeaponType)i;
        }
    }
    return types;
}

void Player::updateHitbox() {
    //currentHitbox = CommonActor::getHitbox(24u, 24u);
    // TODO: Figure out how to use hot/coldspots properly.
    // The sprite is always located relative to the hotspot.
    // The coldspot is usually located at the ground level of the sprite,
    // but for falling sprites for some reason somewhere above the hotspot instead.
    // It is absolutely important that the position of the hitbox stays constant
    // to the hotspot, though; otherwise getting stuck at walls happens all the time.
    currentHitbox = Hitbox(pos + CoordinatePair(0, 8), 24, 24);
}

void Player::endDamagingMove() {
    currentSpecialMove = SPECIAL_MOVE_NONE;
    controllable = true;
    isGravityAffected = true;
    setAnimation(currentAnimation->getAnimationState() & ~AnimState::UPPERCUT & ~AnimState::SIDEKICK & ~AnimState::BUTTSTOMP);
}

void Player::verifyOSDInitialized() {
    if (osd == nullptr) {
        osd = std::make_unique<PlayerOSD>(api, std::dynamic_pointer_cast<Player>(shared_from_this()));
        osd->setWeaponType(currentWeapon, isWeaponPoweredUp[currentWeapon]);
        osd->setAmmo(ammo[currentWeapon]);
        osd->setLives(lives, true);
        osd->setScore(score);
        osd->setHealth(health);
    }
}

void Player::followCarryingPlatform() {
    if (!carryingObject.expired()) {
        if (std::abs(speed.y) > EPSILON || !controllable || !isGravityAffected) {
            carryingObject = std::weak_ptr<MovingPlatform>();
        } else {
            auto platform = carryingObject.lock();
            CoordinatePair delta = platform->getLocationDelta() - CoordinatePair(0.0, 1.0);

            // TODO: disregard the carrying object itself in this collision check to
            // eliminate the need of the correction pixel removed from the delta
            // and to make the ride even smoother (right now the pixel gap is clearly
            // visible when platforms go down vertically)
            if (
                !moveInstantly(delta, false) &&
                !moveInstantly({ 0.0, delta.y }, false)
                ) {
                carryingObject = std::weak_ptr<MovingPlatform>();
            }
        }
    }
}

void Player::updateSpeedBasedAnimation(double lastX) {
    if (controllable) {
        AnimStateT oldState = currentAnimation->getAnimationState();
        AnimStateT newState;
        if (canJump && isActivelyPushing && std::abs(pos.x - lastX - speed.x) > 0.1 && std::abs(externalForce.x) < EPSILON && (isFacingLeft ^ (speed.x > 0))) {
            newState = AnimState::PUSH;
        } else {

            // determine current animation last bits from speeds
            // it's okay to call setAnimation on every tick because it doesn't do
            // anything if the animation is the same as it was earlier

            // only certain ones don't need to be preserved from earlier state, others should be set as expected
            int composite = currentAnimation->getAnimationState() & 0xFFFFBFE0;
            if (std::abs(speed.x) > 3 + EPSILON) {
                // shift-running, speed is more than 3px/frame
                composite += 3;
            } else if (std::abs(speed.x) > 1) {
                // running, speed is between 1px and 3px/frame
                composite += 2;
            } else if (std::abs(speed.x) > EPSILON) {
                // walking, speed is less than 1px/frame (mostly a transition zone)
                composite += 2;
            }

            if (suspendType != SuspendType::SUSPEND_NONE) {
                composite += 12;
            } else {
                if (canJump) {
                    // grounded, no vertical speed
                } else if (speed.y < -EPSILON) {
                    // jumping, ver. speed is negative
                    composite += 4;
                } else {
                    // falling, ver. speed is positive
                    composite += 8;
                }
            }

            newState = AnimStateT(composite);
        }
        setAnimation(newState);

        switch (oldState) {
            case AnimState::RUN:
                if ((newState == AnimState::IDLE) || (newState == AnimState::WALK)) {
                    setTransition(AnimState::TRANSITION_RUN_TO_IDLE, true);
                }
                if (newState == AnimState::DASH) {
                    setTransition(AnimState::TRANSITION_RUN_TO_DASH, true);
                }
                break;
            case AnimState::FALL:
                if (newState == AnimState::IDLE) {
                    setTransition(AnimState::TRANSITION_IDLE_FALL_TO_IDLE, true);
                }
                break;
            case AnimState::IDLE:
                if (newState == AnimState::JUMP) {
                    setTransition(AnimState::TRANSITION_IDLE_TO_IDLE_JUMP, true);
                }
                break;
            case AnimState::SHOOT:
                if (newState == AnimState::IDLE) {
                    setTransition(AnimState::TRANSITION_IDLE_SHOOT_TO_IDLE, true);
                }
                break;
        }

    }
}

void Player::updateCameraPosition() {
    AnimStateT currentState = currentAnimation->getAnimationState();
    // Move camera if up or down held
    if ((currentState & AnimState::CROUCH) > 0) {
        // Down is being held, move camera one unit down
        cameraShiftFramesCount = std::min(128, cameraShiftFramesCount + 1);
    } else if ((currentState & AnimState::LOOKUP) > 0) {
        // Up is being held, move camera one unit up
        cameraShiftFramesCount = std::max(-128, cameraShiftFramesCount - 1);
    } else {
        // Neither is being held, move camera up to 10 units back to equilibrium
        cameraShiftFramesCount = (cameraShiftFramesCount > 0 ? 1 : -1) * std::max(0, std::abs(cameraShiftFramesCount) - 10);
    }

    lightLocation = pos - CoordinatePair(0.0, 10.0);
}

void Player::pushSolidObjects() {
    if (canJump && controllable && isActivelyPushing && std::abs(speed.x) > EPSILON) {
        std::weak_ptr<SolidObject> object;
        if (!(api->isPositionEmpty(currentHitbox + CoordinatePair((speed.x < 0) ? -2.0 : 2.0, 0.0), false, shared_from_this(), object))) {
            auto objectPtr = object.lock();

            if (objectPtr != nullptr) {
                objectPtr->push(speed.x < 0);
            }
        }
    }
}

void Player::checkEndOfSpecialMoves() {
    AnimStateT currentState = currentAnimation->getAnimationState();

    // Buttstomp
    if (currentSpecialMove == SPECIAL_MOVE_BUTTSTOMP && (canJump || suspendType != SuspendType::SUSPEND_NONE)) {
        endDamagingMove();
    }

    // Uppercut
    if (currentSpecialMove == SPECIAL_MOVE_UPPERCUT && !inTransition && ((currentState & (AnimState::UPPERCUT)) > 0) && speed.y > -2 && !canJump) {
        endDamagingMove();
        setTransition(AnimState::TRANSITION_END_UPPERCUT, false);
    }

    // Copter ears
    if ((currentState & (AnimState::COPTER)) > 0) {
        if (canJump || copterFramesLeft == 0 || suspendType != SUSPEND_NONE) {
            setAnimation(currentState & ~AnimState::COPTER);
            if (!isAttachedToPole) {
                isGravityAffected = true;
            }
        } else {
            if (copterFramesLeft > 0) {
                copterFramesLeft--;
            }
        }
    }
}

void Player::checkDestructibleTiles() {
    auto tiles = api->getGameTiles().lock();
    if (tiles == nullptr) {
        return;
    }

    auto tileCollisionHitbox = Hitbox(currentHitbox).extend(-0.5).extend(-speed.x, -speed.y, speed.x, speed.y);

    // Buttstomp/etc. tiles checking
    if (currentSpecialMove != SPECIAL_MOVE_NONE || isSugarRush) {
        uint destroyedCount = tiles->checkSpecialDestructible(tileCollisionHitbox);
        addScore(destroyedCount * 50);

        std::weak_ptr<SolidObject> object;
        if (!(api->isPositionEmpty(tileCollisionHitbox, false, shared_from_this(), object))) {
            {
                auto collider = std::dynamic_pointer_cast<TriggerCrate>(object.lock());
                if (collider != nullptr) {
                    collider->decreaseHealth(1);
                }
            }
            {
                auto collider = std::dynamic_pointer_cast<GenericContainer>(object.lock());
                if (collider != nullptr) {
                    collider->decreaseHealth(1);
                }
            }
            {
                auto collider = std::dynamic_pointer_cast<PowerUpMonitor>(object.lock());
                if (collider != nullptr) {
                    collider->destroyAndApplyToPlayer(std::dynamic_pointer_cast<Player>(shared_from_this()), 1);
                }
            }
        }
    }

    tileCollisionHitbox = Hitbox(currentHitbox).extend(std::abs(speed.x), std::abs(speed.y)).extend(3.0);

    // Speed tiles checking
    if (std::abs(speed.x) > EPSILON || std::abs(speed.y) > EPSILON || isSugarRush) {
        uint destroyedCount = tiles->checkSpecialSpeedDestructible(
            tileCollisionHitbox,
            isSugarRush ? 64.0 : std::max(std::abs(speed.x), std::abs(speed.y)));
        addScore(destroyedCount * 50);
    }

    tiles->checkCollapseDestructible(tileCollisionHitbox);
}

void Player::checkSuspendedStatus() {
    auto tiles = api->getGameTiles().lock();
    if (tiles == nullptr) {
        return;
    }

    AnimStateT currentState = currentAnimation->getAnimationState();
    SuspendType newSuspendState = tiles->getPositionSuspendType(pos - CoordinatePair(0, 5));

    if (newSuspendState != SuspendType::SUSPEND_NONE) {
        suspendType = newSuspendState;
        isGravityAffected = false;

        if (speed.y > 0 && newSuspendState == SuspendType::SUSPEND_VINE) {
            playSound("PLAYER_VINE_ATTACH");
        }

        speed.y = 0;
        externalForce.y = 0;

        if (newSuspendState == SuspendType::SUSPEND_HOOK) {
            speed.x = 0;
        }

        // move downwards until we're on the standard height
        while (tiles->getPositionSuspendType(pos - CoordinatePair(0, 5)) != SuspendType::SUSPEND_NONE) {
            moveInstantly({ 0, 1 }, false, true);
        }
        moveInstantly({ 0, -1 }, false, true);
    } else {
        suspendType = SuspendType::SUSPEND_NONE;
        if ((currentState & (AnimState::BUTTSTOMP | AnimState::COPTER)) == 0 && !isAttachedToPole) {
            isGravityAffected = true;
        }
    }
}

void Player::handleAreaEvents() {
    auto events = api->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    PCEvent e = events->getPositionEvent(pos);
    quint16 p[8];
    events->getPositionParams(pos, p);
    switch (e) {
        case PC_LIGHT_SET:
            assignedView->setLighting(p[0], false);
            break;
        case PC_WARP_ORIGIN:
            {
                if (!inTransition || cancellableTransition) {
                    CoordinatePair c = events->getWarpTarget(p[0]);
                    if (c.x >= 0) {
                        warpToPosition(c);
                    }
                }
            }
            break;
        case PC_MODIFIER_H_POLE:
            if (controllable) {
                initialPoleStage(true);
            }
            break;
        case PC_MODIFIER_V_POLE:
            if (controllable) {
                initialPoleStage(false);
            }
            break;
        case PC_MODIFIER_TUBE:
            {
                endDamagingMove();
                setPlayerTransition(AnimState::DASH | AnimState::JUMP, false, false, SPECIAL_MOVE_NONE);
                isGravityAffected = false;
                speed = { 0, 0 };
                float moveX = (qint16)p[0] * 1.0;
                float moveY = (qint16)p[1] * 1.0;
                if (p[0] != 0) {
                    pos.y = TILE_HEIGHT * pos.tileX() + 8;
                    speed.x = moveX;
                    moveInstantly({ speed.x, 0.0f }, false);
                } else {
                    pos.x = TILE_WIDTH * (pos.tileY() + 0.5);
                    speed.y = moveY;
                    moveInstantly({ 0.0f, speed.y }, false);
                }
            }
            break;
        case PC_AREA_EOL:
            if (!levelExiting) {
                playNonPositionalSound("PLAYER_JAZZ_EOL");
                api->initLevelChange(p[1] == 1 ? NEXT_WARP :
                                        p[0] == 1 ? NEXT_BONUS : NEXT_NORMAL);
            }
            break;
        case PC_AREA_TEXT:
            osd->setLevelText(p[0]);
            if (p[1] != 0) {
                api->getGameEvents().lock()->storeTileEvent(pos.tilePosition(), PC_EMPTY);
            }
            break;
        default:
            break;
    }

    // Check floating from each corner of an extended hitbox
    // Player should not pass from a single tile wide gap if the columns left or right have
    // float events, so checking for a wider box is necessary.
    if (
        (events->getPositionEvent(pos) == PC_AREA_FLOAT_UP) ||
        (events->getPositionEvent(CoordinatePair(currentHitbox.left - 5.0, currentHitbox.top - 5.0)) == PC_AREA_FLOAT_UP) ||
        (events->getPositionEvent(CoordinatePair(currentHitbox.right + 5.0, currentHitbox.top - 5.0)) == PC_AREA_FLOAT_UP) ||
        (events->getPositionEvent(CoordinatePair(currentHitbox.right + 5.0, currentHitbox.bottom + 5.0)) == PC_AREA_FLOAT_UP) ||
        (events->getPositionEvent(CoordinatePair(currentHitbox.left - 5.0, currentHitbox.bottom + 5.0)) == PC_AREA_FLOAT_UP)
        ) {
        if (isGravityAffected) {
            double gravity = (isGravityAffected ? api->getGravity() : 0);

            externalForce.y = gravity * 2;
            speed.y = std::min((float)gravity, speed.y);
        } else {
            speed.y = std::min((float)api->getGravity() * 10, speed.y);
        }
    }
}

void Player::handleActorCollisions() {
    auto collisions = api->findCollisionActors(shared_from_this());
    AnimStateT currentState = currentAnimation->getAnimationState();
    bool removeSpecialMove = false;

    for (const auto& collision : collisions) {
        auto collisionPtr = collision.lock();

        // Different things happen with different actor types

        {
            auto enemy = std::dynamic_pointer_cast<Enemy>(collisionPtr);
            if (enemy != nullptr) {
                if (currentSpecialMove != SPECIAL_MOVE_NONE || isSugarRush) {
                    enemy->decreaseHealth(1);
                    if (isSugarRush) {
                        if (canJump) {
                            speed.y = 3;
                            canJump = false;
                            externalForce.y = 0.6;
                        }
                        speed.y *= -.5;
                    }
                    if ((currentState & AnimState::BUTTSTOMP) > 0) {
                        removeSpecialMove = true;
                        speed.y *= -.5;
                    }
                } else {
                    if (enemy->hurtsPlayer()) {
                        takeDamage(4 * (pos.x > enemy->getPosition().x ? 1 : -1));
                    }
                }
                continue;
            }
        }

        {
            auto sp = std::dynamic_pointer_cast<SavePoint>(collisionPtr);
            if (sp != nullptr) {
                sp->activateSavePoint();
                continue;
            }
        }

        {
            auto spring = std::dynamic_pointer_cast<Spring>(collisionPtr);
            if (spring != nullptr) {
                removeSpecialMove = true;
                sf::Vector2f params = spring->activate();
                short sign = ((params.x + params.y) > EPSILON ? 1 : -1);
                if (std::abs(params.x) > EPSILON) {
                    speed.x = (4 + std::abs(params.x)) * sign;
                    externalForce.x = params.x;
                    setPlayerTransition(AnimState::DASH | AnimState::JUMP, true, false, SPECIAL_MOVE_NONE);
                } else if (std::abs(params.y) > EPSILON) {
                    speed.y = (4 + std::abs(params.y)) * sign;
                    externalForce.y = -params.y;
                    setPlayerTransition(sign == -1 ? AnimState::TRANSITION_SPRING : AnimState::BUTTSTOMP, true, false, SPECIAL_MOVE_NONE);
                } else {
                    continue;
                }
                canJump = false;
                continue;
            }
        }

        {
            auto collectible = std::dynamic_pointer_cast<Collectible>(collisionPtr);
            if (collectible != nullptr) {
                collectible->collect(std::dynamic_pointer_cast<Player>(shared_from_this()));
                collisionPtr->deleteFromEventMap();
                api->removeActor(collisionPtr);
            }
        }

        if (!inTransition || cancellableTransition) {
            auto collider = std::dynamic_pointer_cast<BonusWarp>(collisionPtr);
            if (collider != nullptr) {
                quint16 owed = collider->getCost();
                if (owed <= getCoinsTotalValue()) {
                    while (owed >= 5 && collectedCoins[1] > 0) {
                        owed -= 5;
                        collectedCoins[1]--;
                    }
                    collectedCoins[0] -= owed;

                    setupOSD(OSD_COIN_SILVER, getCoinsTotalValue());
                    warpToPosition(collider->getWarpTarget());
                } else {
                    setupOSD(OSD_BONUS_WARP_NOT_ENOUGH_COINS, owed - getCoinsTotalValue());
                }
            }
        }

        if (currentSpecialMove != SPECIAL_MOVE_NONE || isSugarRush) {
            auto collider = std::dynamic_pointer_cast<TurtleShell>(collisionPtr);
            if (collider != nullptr) {
                collider->decreaseHealth(10);
                continue;
            }
        }
    }

    if (removeSpecialMove) {
        endDamagingMove();
    }
}

bool Player::setPlayerTransition(AnimStateT state, bool cancellable, bool remove_control, SpecialMoveType set_special,
    AnimationCallbackFunc callback) {
    if (remove_control) {
        controllable = false;
    }
    currentSpecialMove = set_special;

    return CommonActor::setTransition(state, cancellable, callback);
}

void Player::onHitFloorHook() {
    auto events = api->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    if (events->isPositionHurting(pos + CoordinatePair(0, 24))) {
        takeDamage(speed.x / 4);
    } else {
        if (!canJump) {
            playSound("COMMON_LAND");
        }
    }
}

void Player::onHitCeilingHook() {
    auto events = api->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    if (events->isPositionHurting(pos - CoordinatePair(0, 4))) {
        takeDamage(speed.x / 4);
    }
}

void Player::onHitWallHook() {
    auto events = api->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    if (events->isPositionHurting(pos + CoordinatePair((speed.x > 0 ? 1 : -1) * 16, 0))) {
        takeDamage(speed.x / 4);
    }
}

void Player::takeDamage(double pushForce) {
    if (!isInvulnerable && !levelExiting) {
        health = static_cast<unsigned>(std::max(static_cast<int>(health - 1), 0));
        externalForce.x = pushForce;
        internalForce.y = 0;
        speed = { 0, -6.5 };
        canJump = false;
        setPlayerTransition(AnimState::HURT, false, true, SPECIAL_MOVE_NONE, [this]() {
            controllable = true;
        });
        setInvulnerability(210u, true);
        playSound("PLAYER_JAZZ_HURT");
        osd->setHealth(health);
    }
}

void Player::setToOwnViewCenter() {
    int shift_offset = 0;

    if (std::abs(cameraShiftFramesCount) > 48) {
        shift_offset = (std::abs(cameraShiftFramesCount) - 48) * (cameraShiftFramesCount > 0 ? 1 : -1);
    }
    
    assignedView->centerView(
        std::max(
            assignedView->getViewWidth() / 2.0,
            std::min(
                TILE_WIDTH * api->getLevelTileWidth() - assignedView->getViewWidth()  / 2.0,
                (double)qRound(pos.x) + (assignedView->getViewWidth() % 2 == 0 ? 0 : 0.5)
            )
        ), std::max(
            assignedView->getViewHeight() / 2.0,
            std::min(
                TILE_HEIGHT * api->getLevelTileHeight() - assignedView->getViewHeight() / 2.0,
                (double)qRound(pos.y + shift_offset - 15) + (assignedView->getViewHeight() % 2 == 0 ? 0 : 0.5)
            )
        )
    );
}

bool Player::deactivate(const TileCoordinatePair&, int) {
    // A player can never be deactivated
    return false;
}

unsigned Player::getLives() {
    return lives;
}

bool Player::getPowerUp(WeaponType type) const {
    if (type > (WEAPONCOUNT - 1)) { return false; }
    return isWeaponPoweredUp[static_cast<uint>(type)];
}

void Player::initialPoleStage(bool horizontal) {
    bool positive;
    if (horizontal) {
        positive = (speed.x > 0);
    } else {
        positive = (speed.y > 0);
    }

    pos.x = TILE_WIDTH * (pos.tileX() + 0.5);
    pos.y = TILE_HEIGHT * (pos.tileY() + 0.5);
    speed = { 0, 0 };
    externalForce = { 0, 0 };
    internalForce = { 0, 0 };
    canJump = false;
    isGravityAffected = false;
    controllable = false;
    isAttachedToPole = true;

    AnimStateT poleAnim = horizontal ? AnimState::TRANSITION_POLE_H_SLOW : AnimState::TRANSITION_POLE_V_SLOW;

    setPlayerTransition(poleAnim, false, true, SPECIAL_MOVE_NONE, [this, horizontal, positive]() {
        nextPoleStage(horizontal, positive, 2);
    });
}

void Player::nextPoleStage(bool horizontal, bool positive, ushort stagesLeft) {
    if (stagesLeft > 0) {
        AnimStateT poleAnim = horizontal ? AnimState::TRANSITION_POLE_H : AnimState::TRANSITION_POLE_V;

        setPlayerTransition(poleAnim, false, true, SPECIAL_MOVE_NONE, [this, horizontal, positive, stagesLeft]() {
            nextPoleStage(horizontal, positive, stagesLeft - 1);
        });
    } else {
        int mp = positive ? 1 : -1;
        if (horizontal) {
            speed.x = 10 * mp;
            moveInstantly({ speed.x, 0.0f }, false, true);
            externalForce.x = 10 * mp;
            isFacingLeft = !positive;
        } else {
            moveInstantly({ 0.0, mp * 16.0 }, false, true);
            speed.y = 5 * mp;
            externalForce.y = -2 * mp;
        }
        controllable = true;
        isGravityAffected = true;
        isAttachedToPole = false;
    }
}

PlayerCarryOver Player::prepareLevelCarryOver() {
    PlayerCarryOver o;
    o.lives = lives;
    o.fastfires = fastfires;
    for (int i = 0; i < WEAPONCOUNT; ++i) {
        o.ammo[i] = ammo[i];
        o.poweredUp[i] = isWeaponPoweredUp[i];
    }
    o.score = score;
    o.foodCounter = foodCounter;
    o.currentWeapon = currentWeapon;
    return o;
}

void Player::receiveLevelCarryOver(ExitType exitType, const PlayerCarryOver& o) {
    lives = o.lives;
    fastfires = o.fastfires;
    for (int i = 0; i < WEAPONCOUNT; ++i) {
        ammo[i] = o.ammo[i];
        isWeaponPoweredUp[i] = o.poweredUp[i];
    }
    score = o.score;
    foodCounter = o.foodCounter;
    currentWeapon = o.currentWeapon;

    if (exitType == NEXT_WARP) {
        playNonPositionalSound("COMMON_WARP_OUT");
        isGravityAffected = false;
        setPlayerTransition(AnimState::TRANSITION_WARP_END, false, true, SPECIAL_MOVE_NONE, [this]() {
            isInvulnerable = false;
            isGravityAffected = true;
            controllable = true;
        });
    }
}

void Player::addScore(unsigned points) {
    score = std::min(99999999ul, score + points);
    osd->setScore(score);
}

void Player::setCarryingPlatform(std::weak_ptr<MovingPlatform> platform) {
    if (speed.y < -EPSILON) {
        return;
    }

    carryingObject = platform;
    canJump = true;
    internalForce.y = 0;
    speed.y = 0;
}

void Player::setView(std::shared_ptr<GameView> view) {
    assignedView = view;
}

void Player::setExiting(ExitType e) {
    levelExiting = true;
    if (e == NEXT_WARP) {
        addTimer(285u, false, [this]() {
            isFacingLeft = false;
            setPlayerTransition(AnimState::TRANSITION_WARP, false, true, SPECIAL_MOVE_NONE, [this]() {
                isInvisible = true;
            });
            playNonPositionalSound("COMMON_WARP_IN");
        });
    } else {
        addTimer(255u, false, [this]() {
            isFacingLeft = false;
            setPlayerTransition(AnimState::TRANSITION_END_OF_LEVEL, false, true, SPECIAL_MOVE_NONE, [this]() {
                isInvisible = true;
            });
            playNonPositionalSound("PLAYER_EOL_1");
        });
        addTimer(335u, false, [this]() {
            playNonPositionalSound("PLAYER_EOL_2");
        });
    }

    osd->initLevelCompletedOverlay(collectedGems[0], collectedGems[1], collectedGems[2], collectedGems[3]);
}

void Player::setupOSD(OSDMessageType type, int param) {
    switch (type) {
        case OSD_GEM_RED: 
            osd->setMessage(OSD_GEM_RED, getGemsTotalValue());
            break;
        case OSD_GEM_GREEN:
            osd->setMessage(OSD_GEM_GREEN, collectedGems[1]);
            break;
        case OSD_GEM_BLUE:
            osd->setMessage(OSD_GEM_BLUE, collectedGems[2]);
            break;
        case OSD_GEM_PURPLE:
            osd->setMessage(OSD_GEM_PURPLE, collectedGems[3]);
            break;
        case OSD_COIN_SILVER: 
            osd->setMessage(OSD_COIN_SILVER, getCoinsTotalValue());
            break;
        case OSD_COIN_GOLD: 
            osd->setMessage(OSD_COIN_GOLD, collectedCoins[1]);
            break;
        case OSD_BONUS_WARP_NOT_ENOUGH_COINS: 
            osd->setMessage(OSD_BONUS_WARP_NOT_ENOUGH_COINS, param);
            break;
        case OSD_CUSTOM_TEXT:
            // TODO
            break;
        case OSD_NONE:
            break;
    }
}

uint Player::getGemsTotalValue() {
    return collectedGems[0] + collectedGems[1] * 5 + collectedGems[2] * 10;
}

uint Player::getCoinsTotalValue() {
    return collectedCoins[0] + collectedCoins[1] * 5;
}

void Player::warpToPosition(const CoordinatePair& pos) {
    setPlayerTransition(AnimState::TRANSITION_WARP, false, true, SPECIAL_MOVE_NONE, [this, pos]() {
        auto events = api->getGameEvents().lock();
        if (events == nullptr) {
            return;
        }

        moveInstantly(pos, true, true);
        playSound("COMMON_WARP_OUT");

        setPlayerTransition(AnimState::TRANSITION_WARP_END, false, true, SPECIAL_MOVE_NONE, [this]() {
            isInvulnerable = false;
            isGravityAffected = true;
            controllable = true;
        });
    });

    isInvulnerable = true;
    isGravityAffected = false;
    speed = { 0, 0 };
    externalForce = { 0, 0 };
    internalForce = { 0, 0 };
    playSound("COMMON_WARP_IN");
}

template<typename T>
std::shared_ptr<T> Player::fireWeapon(bool poweredUp) {
    auto weakPtr = std::dynamic_pointer_cast<Player>(shared_from_this());
    bool lookup = ((currentAnimation->getAnimationState() & AnimState::LOOKUP) > 0);
    auto animation = currentAnimation->getAnimation();
    int fireX = (animation->gunspot.x - animation->hotspot.x) * (isFacingLeft ? -1 : 1);
    int fireY =  animation->gunspot.y - animation->hotspot.y;

    auto newAmmo = std::make_shared<T>(ActorInstantiationDetails(api, pos + CoordinatePair(fireX, fireY)), weakPtr, speed.x, isFacingLeft, lookup, poweredUp);
    api->addActor(newAmmo);
    return newAmmo;
}

const uint Player::SUGAR_RUSH_THRESHOLD = 100;
const double Player::MAX_DASHING_SPEED = 9.0;
const double Player::MAX_RUNNING_SPEED = 3.0;
const double Player::ACCELERATION = 0.2;
const double Player::DECELERATION = 0.25;
