#include "Player.h"
#include "../gamestate/TileMap.h"
#include "../gamestate/EventMap.h"
#include "SolidObject.h"
#include "TriggerCrate.h"
#include "enemy/Enemy.h"
#include "collectible/Collectible.h"
#include "SavePoint.h"
#include "Spring.h"
#include "weapon/AmmoBlaster.h"
#include "weapon/AmmoBouncer.h"
#include "weapon/AmmoToaster.h"

Player::Player(std::shared_ptr<CarrotQt5> root, double x, double y) : CommonActor(root, x, y, false), 
    weaponCooldown(0), character(CHAR_JAZZ), controllable(true), isUsingDamagingMove(false), 
    cameraShiftFramesCount(0), copterFramesLeft(0), fastfires(0), foodCounter(0), isSugarRush(false),
    poleSpinCount(0), poleSpinDirectionPositive(false), toasterAmmoSubticks(10), score(0) {
    loadResources("Interactive/PlayerJazz");

    maxHealth = 5;
    health = 5;
    lives = 3;
    currentWeapon = WEAPON_BLASTER;
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
        Qt::Key::Key_Control,
        Qt::Key::Key_Space,
        Qt::Key::Key_Shift,
        Qt::Key::Key_Enter
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
        if (controllable && canJump && std::abs(speedX < 1e-6)) {
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
            if (canJump && std::abs(speedX < 1e-6)) {
                setAnimation(AnimState::CROUCH);
            } else {
                if (suspendType != SuspendType::SUSPEND_NONE) {
                    posY += 10;
                    suspendType = SuspendType::SUSPEND_NONE;
                } else {
                    controllable = false;
                    speedX = 0;
                    speedY = 0;
                    internalForceY = 0;
                    externalForceY = 0;
                    isGravityAffected = false;
                    isUsingDamagingMove = true;
                    setAnimation(AnimState::BUTTSTOMP);
                    setTransition(AnimState::TRANSITION_BUTTSTOMP_START, true, false, false, &Player::delayedButtstompStart);
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
                    setTransition(AnimState::TRANSITION_UPPERCUT_A, true, true, true, &Player::delayedUppercutStart);
                } else {
                    if (speedY > 0 && !canJump) {
                        isGravityAffected = false;
                        speedY = 1.5;
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
    if (!controllable) {
        return;
    }

    const AnimStateT currentState = currentAnimation->getAnimationState();

    if (e.contains(controls.leftButton) || e.contains(controls.rightButton)) {
        isFacingLeft = !e.contains(controls.rightButton);

        if (suspendType == SuspendType::SUSPEND_NONE && (e.contains(controls.dashButton))) {
            speedX = std::max(std::min(speedX + 0.2 * (isFacingLeft ? -1 : 1), 9.0), -9.0);
        } else if (suspendType != SuspendType::SUSPEND_HOOK) {
            speedX = std::max(std::min(speedX + 0.2 * (isFacingLeft ? -1 : 1), 3.0), -3.0);
        }

    } else {
        speedX = std::max((abs(speedX) - 0.25), 0.0) * (speedX < -1e-6 ? -1 : 1);
    }

    if (e.contains(controls.jumpButton)) {
        if (suspendType != SuspendType::SUSPEND_NONE) {
            posY -= 5;
            canJump = true;
        }
        if (canJump && ((currentState & AnimState::UPPERCUT) == 0) && !e.contains(controls.downButton)) {
            internalForceY = 1.2;
            speedY = -3 - std::max(0.0, (std::abs(speedX) - 4.0) * 0.3);
            canJump = false;
            setAnimation(currentState & (~AnimState::LOOKUP & ~AnimState::CROUCH));
            playSound("COMMON_JUMP");
            carryingObject = std::weak_ptr<MovingPlatform>();
        }
    } else {
        if (internalForceY > 0) {
            internalForceY = 0;
        }
    }

    processAllControlHeldEventsDefaultHandler(e);
}

void Player::processControlHeldEvent(const ControlEvent& e) {
    const AnimStateT currentState = currentAnimation->getAnimationState();

    if (e.first == controls.fireButton) {
        setAnimation(currentState | AnimState::SHOOT);
        if (weaponCooldown == 0) {
            switch (currentWeapon) {
                case WEAPON_BLASTER:
                {
                    auto newAmmo = fireWeapon<AmmoBlaster>();
                    weaponCooldown = std::max(0, 40 - 3 * fastfires);
                    playSound("WEAPON_BLASTER_JAZZ");
                    break;
                }
                case WEAPON_BOUNCER:
                {
                    auto newAmmo = fireWeapon<AmmoBouncer>();
                    weaponCooldown = 25;
                    break;
                }
                case WEAPON_TOASTER:
                {
                    auto newAmmo = fireWeapon<AmmoToaster>();
                    weaponCooldown = 3;
                    break;
                }
                case WEAPON_TNT:
                    // do nothing, TNT can only be placed by keypress, not by holding space down
                    break;
                case WEAPON_FREEZER:
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
    // Initialize these ASAP
    if (osd == nullptr) {
        osd = std::make_unique<PlayerOSD>(root, std::dynamic_pointer_cast<Player>(shared_from_this()));
        osd->setWeaponType(currentWeapon, isWeaponPoweredUp[currentWeapon]);
        osd->setAmmo(ammo[currentWeapon]);
        osd->setLives(lives);
        osd->setScore(score);
        osd->setHealth(health);
    }

    auto tiles = root->getGameTiles().lock();
    AnimStateT currentState = currentAnimation->getAnimationState();

    // Check for pushing
    if (canJump && controllable && std::abs(speedX) > 1e-6) {
        std::weak_ptr<SolidObject> object;
        if (!(root->isPositionEmpty(getHitbox() + CoordinatePair(speedX < 0 ? -1 : 1, 0), false, shared_from_this(), object))) {
            auto objectPtr = object.lock();

            if (objectPtr != nullptr) {
                objectPtr->push(speedX < 0);
                setAnimation(currentState | AnimState::PUSH);
            } else {
                setAnimation(currentState & ~AnimState::PUSH);
            }
        } else {
            setAnimation(currentState & ~AnimState::PUSH);
        }
    } else {
        setAnimation(currentState & ~AnimState::PUSH);
    }

    if (!carryingObject.expired()) {
        if (speedY > 1e-6) {
            carryingObject = std::weak_ptr<MovingPlatform>();
        } else {
            auto platform = carryingObject.lock();
            CoordinatePair delta = platform->getLocationDelta();
            Hitbox currentHitbox = getHitbox();

            if (root->isPositionEmpty(currentHitbox + CoordinatePair(delta.x, delta.y), false, shared_from_this())) {
                posX += delta.x;
                posY += delta.y;
            } else if (root->isPositionEmpty(currentHitbox + CoordinatePair(0.0, delta.y), false, shared_from_this())) {
                posY += delta.y;
            } else {
                carryingObject = std::weak_ptr<MovingPlatform>();
            }
        }

    }

    CommonActor::tickEvent();
    currentState = currentAnimation->getAnimationState();
    short sign = ((speedX + externalForceX) > 1e-6) ? 1 : (((speedX + externalForceX) < -1e-6) ? -1 : 0);
    double gravity = (isGravityAffected ? root->gravity : 0);


    if (tiles != nullptr) {
        // Check if hitting a vine
        SuspendType state = tiles->getPosSuspendState(posX, posY - 5);

        if (state != SuspendType::SUSPEND_NONE) {
            suspendType = state;
            isGravityAffected = false;

            if (speedY > 0 && state == SuspendType::SUSPEND_VINE) {
                playSound("PLAYER_VINE_ATTACH");
            }

            speedY = 0;
            externalForceY = 0;

            if (state == SuspendType::SUSPEND_HOOK) {
                speedX = 0;
            }

            // move downwards until we're on the standard height
            while (tiles->getPosSuspendState(posX, posY - 5) != SuspendType::SUSPEND_NONE) {
                posY += 1;
            }
            posY -= 1;
        } else {
            suspendType = SuspendType::SUSPEND_NONE;
            if (((currentState & (AnimState::BUTTSTOMP | AnimState::COPTER)) == 0) && (poleSpinCount == 0)) {
                isGravityAffected = true;
            }
        }

        auto tileCollisionHitbox = getHitbox().extend(2 + std::abs(speedX), 2 + std::abs(speedY));

        // Buttstomp/etc. tiles checking
        if (isUsingDamagingMove || isSugarRush) {
            uint destroyedCount = tiles->checkSpecialDestructible(tileCollisionHitbox);
            addScore(destroyedCount * 50);

            std::weak_ptr<SolidObject> object;
            if (!(root->isPositionEmpty(tileCollisionHitbox, false, shared_from_this(), object))) {
                auto triggerCrate = std::dynamic_pointer_cast<TriggerCrate>(object.lock());
                if (triggerCrate != nullptr) {
                    triggerCrate->decreaseHealth(1);
                }
            }
        }

        // Speed tiles checking
        if (std::abs(speedX) > 1e-6 || std::abs(speedY) > 1e-6 || isSugarRush) {
            uint destroyedCount = tiles->checkSpecialSpeedDestructible(tileCollisionHitbox,
                isSugarRush ? 64.0 : std::max(std::abs(speedX), std::abs(speedY)));
            addScore(destroyedCount * 50);
        }

        tiles->checkCollapseDestructible(tileCollisionHitbox.add(0, 2));
    }

    // check if buttstomp ended
    if (canJump && (currentState & AnimState::BUTTSTOMP) > 0 || (isUsingDamagingMove && suspendType != SuspendType::SUSPEND_NONE)) {
        setAnimation(currentState & ~AnimState::BUTTSTOMP);
        isUsingDamagingMove = false;
        controllable = true;
    }

    // check if copter ears ended
    if ((currentState & (AnimState::COPTER)) > 0) {
        if (canJump || copterFramesLeft == 0 || suspendType != SUSPEND_NONE) {
            isGravityAffected = true;
            setAnimation(currentState & ~AnimState::COPTER);
        } else {
            if (copterFramesLeft > 0) {
                copterFramesLeft--;
            }
        }
    }

    // check if uppercut ended
    if (((currentState & (AnimState::UPPERCUT)) > 0) && speedY > -2 && !canJump) {
        endDamagingMove();
        setTransition(AnimState::TRANSITION_END_UPPERCUT, false);
    }
    
    auto events = root->getGameEvents().lock();
    if (events != nullptr) {
        PCEvent e = events->getPositionEvent(posX, posY);
        quint16 p[8];
        events->getPositionParams(posX, posY, p);
        switch (e) {
            case PC_LIGHT_SET:
                assignedView->setLighting(p[0], false);
                break;
            case PC_WARP_ORIGIN:
            {
                if (!inTransition || cancellableTransition) {
                    CoordinatePair c = events->getWarpTarget(p[0]);
                    if (c.x >= 0) {
                        setTransition(AnimState::TRANSITION_WARP, false, true, false, &Player::endWarpTransition);
                        isInvulnerable = true;
                        isGravityAffected = false;
                        speedX = 0;
                        speedY = 0;
                        externalForceX = 0;
                        externalForceY = 0;
                        internalForceY = 0;
                        playSound("COMMON_WARP_IN");
                    }
                }
            }
            break;
            case PC_MODIFIER_H_POLE:
                if (poleSpinCount == 0) {
                    posY = std::floor(qRound(posY) / 32) * 32 + 16;
                    setTransition(AnimState::TRANSITION_POLE_H_SLOW, false, true, false, &Player::endHPoleTransition);
                    poleSpinDirectionPositive = (speedX > 0);
                    posX = std::floor(qRound(posX) / 32) * 32 + 16;
                    poleSpinCount = 3;
                    speedX = 0;
                    speedY = 0;
                    externalForceX = 0;
                    externalForceY = 0;
                    internalForceY = 0;
                    canJump = false;
                    isGravityAffected = false;
                }
                break;
            case PC_MODIFIER_V_POLE:
                if (poleSpinCount == 0) {
                    posY = std::floor(qRound(posY) / 32) * 32 + 16;
                    setTransition(AnimState::TRANSITION_POLE_V_SLOW, false, true, false, &Player::endVPoleTransition);
                    poleSpinDirectionPositive = (speedY > 0);
                    posX = std::floor(qRound(posX) / 32) * 32 + 16;
                    poleSpinCount = 3;
                    speedX = 0;
                    speedY = 0;
                    externalForceX = 0;
                    externalForceY = 0;
                    internalForceY = 0;
                    canJump = false;
                    isGravityAffected = false;
                }
                break;
            case PC_MODIFIER_TUBE:
                {
                    endDamagingMove();
                    setTransition(AnimState::DASH | AnimState::JUMP, false, false, false);
                    isGravityAffected = false;
                    speedX = 0;
                    speedY = 0;
                    float moveX = (qint16)p[0] * 1.0;
                    float moveY = (qint16)p[1] * 1.0;
                    if (p[0] != 0) {
                        posY = std::floor(posY / 32) * 32 + 8;
                        speedX = moveX;
                        posX += speedX;
                    } else {
                        posX = std::floor(posX / 32) * 32 + 16;
                        speedY = moveY;
                        posY += speedY;
                    }
                }
                break;
            case PC_AREA_EOL:
                if (controllable) {
                    playNonPositionalSound("PLAYER_JAZZ_EOL");
                    root->initLevelChange(NEXT_NORMAL);
                }
                controllable = false;
                break;
        }

        // Check floating from each corner of an extended hitbox
        // Player should not pass from a single tile wide gap if the columns left or right have
        // float events, so checking for a wider box is necessary.
        Hitbox hitbox = getHitbox();
        if (
            (events->getPositionEvent(posX,               posY               ) == PC_AREA_FLOAT_UP) ||
            (events->getPositionEvent(hitbox.left  - 5.0, hitbox.top    - 5.0) == PC_AREA_FLOAT_UP) ||
            (events->getPositionEvent(hitbox.right + 5.0, hitbox.top    - 5.0) == PC_AREA_FLOAT_UP) ||
            (events->getPositionEvent(hitbox.right + 5.0, hitbox.bottom + 5.0) == PC_AREA_FLOAT_UP) ||
            (events->getPositionEvent(hitbox.left  - 5.0, hitbox.bottom + 5.0) == PC_AREA_FLOAT_UP)
            ) {
            if (isGravityAffected) {
                externalForceY = gravity * 2;
                speedY = std::min(gravity, speedY);
            } else {
                speedY = std::min(root->gravity * 10, speedY);
            }
        }
    }
    
    // reduce player timers for certain things:
    // Weapon cooldown
    if (weaponCooldown > 0) {
        weaponCooldown--;
    }
    
    // Move camera if up or down held
    if ((currentState & AnimState::CROUCH) > 0) {
        // Down is being held, move camera one unit down
        cameraShiftFramesCount = std::min(128, cameraShiftFramesCount + 1);
    } else if ((currentState & AnimState::LOOKUP) > 0) {
        // Up is being held, move camera one unit up
        cameraShiftFramesCount = std::max(-128, cameraShiftFramesCount - 1);
    } else {
        // Neither is being held, move camera up to 10 units back to equilibrium
        cameraShiftFramesCount = (cameraShiftFramesCount > 0 ? 1 : -1) * std::max(0, abs(cameraShiftFramesCount) - 10);
    }

    auto collisions = root->findCollisionActors(shared_from_this());
    for (const auto& collision : collisions) {
        auto collisionPtr = collision.lock();

        // Different things happen with different actor types

        {
            auto enemy = std::dynamic_pointer_cast<Enemy>(collisionPtr);
            if (enemy != nullptr) {
                if (isUsingDamagingMove || isSugarRush) {
                    enemy->decreaseHealth(1);
                    if (isSugarRush) {
                        if (canJump) {
                            speedY = 3;
                            canJump = false;
                            externalForceY = 0.6;
                        }
                        speedY *= -.5;
                    }
                    if ((currentState & AnimState::BUTTSTOMP) > 0) {
                        setAnimation(currentState & ~AnimState::BUTTSTOMP);
                        isUsingDamagingMove = false;
                        controllable = true;
                        speedY *= -.5;
                    }
                } else {
                    if (enemy->hurtsPlayer()) {
                        takeDamage(4 * (posX > enemy->getPosition().x ? 1 : -1));
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
                sf::Vector2f params = spring->activate();
                endDamagingMove();
                short sign = ((params.x + params.y) > 1e-6 ? 1 : -1);
                if (abs(params.x) > 1e-6) {
                    speedX = (4 + abs(params.x)) * sign;
                    externalForceX = params.x;
                    setTransition(AnimState::DASH | AnimState::JUMP, true, false, false);
                } else {
                    speedY = (4 + abs(params.y)) * sign;
                    externalForceY = -params.y;
                    setTransition(sign == -1 ? AnimState::TRANSITION_SPRING : AnimState::BUTTSTOMP, true, false, false);
                }
                canJump = false;
                continue;
            }
        }

        auto collectible = std::dynamic_pointer_cast<Collectible>(collisionPtr);
        if (collectible != nullptr) {
            collectible->collect(std::dynamic_pointer_cast<Player>(shared_from_this()));
            collisionPtr->deleteFromEventMap();
            root->removeActor(collisionPtr);
        }
    }
}

unsigned Player::getHealth() {
    return health;
}

void Player::drawUIOverlay() {
    osd->drawOSD(assignedView);

    auto canvas = assignedView->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

#ifdef CARROT_DEBUG
    if (root->dbgOverlaysActive) {
        BitmapString::drawString(canvas, root->getFont(), "P1: " + QString::number(posX) + "," + QString::number(posY), 6, 86);
        BitmapString::drawString(canvas, root->getFont(), "  Hsp " + QString::number(speedX), 6, 116);
        BitmapString::drawString(canvas, root->getFont(), "  Vsp " + QString::number(speedY), 6, 146);
    }
#endif
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

void Player::debugHealth() {
    health = 5;
}

void Player::debugAmmo() {
    std::fill_n(ammo, WEAPONCOUNT, 999);
}

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

    collectedGems[(uint)type]++;
    setupOSD((OSDMessageType)((uint)OSD_GEM_RED + (uint)type));
}

void Player::addCoins(CoinType type, unsigned amount) {
    if ((uint)type >= 2) {
        return;
    }

    playSound("PLAYER_PICKUP_COIN");
    collectedCoins[(uint)type]++;
    setupOSD((OSDMessageType)((uint)OSD_COIN_SILVER + (uint)type));
}

void Player::addFastFire(unsigned amount) {
    fastfires = std::min(fastfires + 1, 10);
    playSound("PLAYER_PICKUP_AMMO");
}

void Player::consumeFood(const bool& isDrinkable) {
    foodCounter += 1;
    if (foodCounter >= SUGAR_RUSH_THRESHOLD) {
        foodCounter = foodCounter % SUGAR_RUSH_THRESHOLD;
        if (!isSugarRush) {
            auto soundSystem = root->getSoundSystem().lock();
            if (soundSystem != nullptr) {
                soundSystem->pauseMusic();
                playNonPositionalSound("PLAYER_SUGAR_RUSH");
            }

            isSugarRush = true;
            osd->setSugarRushActive();
            addTimer(21.548 * 70.0, false, [this]() {
                isSugarRush = false;
                auto soundSystem = root->getSoundSystem().lock();
                if (soundSystem != nullptr) {
                    soundSystem->resumeMusic();
                }
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
        setTransition(AnimState::TRANSITION_DEATH, false, true, false, &Player::deathRecovery);
    }
    return false;
}

void Player::deathRecovery(std::shared_ptr<AnimationInstance> animation) {
    if (lives > 0) {
        lives--;

        // Return us to the last save point
        root->loadSavePoint();

        // Reset health and remove one life
        health = maxHealth;
        osd->setLives(lives);
        osd->setHealth(maxHealth);

        // Negate all possible movement effects etc.
        transition->clearCallback();
        inTransition = false;
        canJump = false;
        externalForceX = 0;
        externalForceY = 0;
        internalForceY = 0;
        speedX = 0;
        speedY = 0;
        controllable = true;
        
        // remove fast fires
        fastfires = 0;
    } else {
        // TODO: game over handling
    }
}

Hitbox Player::getHitbox() {
    //return CommonActor::getHitbox(24u, 24u);
    // TODO: Figure out how to use hot/coldspots properly.
    // The sprite is always located relative to the hotspot.
    // The coldspot is usually located at the ground level of the sprite,
    // but for falling sprites for some reason somewhere above the hotspot instead.
    // It is absolutely important that the position of the hitbox stays constant
    // to the hotspot, though; otherwise getting stuck at walls happens all the time.
    return Hitbox(CoordinatePair(posX, posY + 8), 24, 24);
}

void Player::endDamagingMove() {
    isUsingDamagingMove = false;
    controllable = true;
    isGravityAffected = true;
    setAnimation(currentAnimation->getAnimationState() & ~AnimState::UPPERCUT & ~AnimState::SIDEKICK & ~AnimState::BUTTSTOMP);
}

void Player::returnControl() {
    controllable = true;
}

void Player::delayedUppercutStart(std::shared_ptr<AnimationInstance> animation) {
    externalForceY = 1.5;
    speedY = -2;
    canJump = false;
    setTransition(AnimState::TRANSITION_UPPERCUT_B, true, true, true);
}

void Player::delayedButtstompStart(std::shared_ptr<AnimationInstance> animation) {
    speedY = 9;
    setAnimation(AnimState::BUTTSTOMP);
    playSound("PLAYER_BUTTSTOMP", 1.0f, 0.0f, 0.8f);
}

bool Player::setTransition(AnimStateT state, bool cancellable, bool remove_control, bool set_special, 
    void(Player::*callback)(std::shared_ptr<AnimationInstance> animation)) {
    if (remove_control) {
        controllable = false;
    }
    if (set_special) {
        isUsingDamagingMove = true;
    }

    return CommonActor::setTransition(state, cancellable, static_cast<AnimationCallbackFunc>(callback));
}

void Player::onHitFloorHook() {
    auto events = root->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    if (events->isPosHurting(posX, posY + 24)) {
        takeDamage(speedX / 4);
    } else {
        if (!canJump) {
            playSound("COMMON_LAND");
        }
    }
}

void Player::onHitCeilingHook() {
    auto events = root->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    if (events->isPosHurting(posX, posY - 4)) {
        takeDamage(speedX / 4);
    }
}

void Player::onHitWallHook() {
    auto events = root->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    if (events->isPosHurting(posX + (speedX > 0 ? 1 : -1) * 16, posY)) {
        takeDamage(speedX / 4);
    }
}

void Player::takeDamage(double pushForce) {
    if (!isInvulnerable) {
        health = static_cast<unsigned>(std::max(static_cast<int>(health - 1), 0));
        externalForceX = pushForce;
        internalForceY = 0;
        speedY = -6.5;
        speedX = 0;
        canJump = false;
        setTransition(AnimState::HURT, false, true, false, &Player::endHurtTransition);
        setInvulnerability(210u, true);
        playSound("PLAYER_JAZZ_HURT");
        osd->setHealth(health);
    }
}

void Player::setToViewCenter() {
    int shift_offset = 0;
    auto tiles = root->getGameTiles().lock();
    if (tiles == nullptr) {
        return;
    }

    if (abs(cameraShiftFramesCount) > 48) {
        shift_offset = (abs(cameraShiftFramesCount) - 48) * (cameraShiftFramesCount > 0 ? 1 : -1);
    }
    
    assignedView->centerView(
        std::max(
            assignedView->getViewWidth() / 2.0,
            std::min(
                32.0 * (tiles->getLevelWidth()  + 1) - assignedView->getViewWidth()  / 2.0,
                (double)qRound(posX) + (assignedView->getViewWidth() % 2 == 0 ? 0 : 0.5)
            )
        ), std::max(
            assignedView->getViewHeight() / 2.0,
            std::min(
                32.0 * (tiles->getLevelHeight() + 1) - assignedView->getViewHeight() / 2.0,
                (double)qRound(posY + shift_offset - 15) + (assignedView->getViewHeight() % 2 == 0 ? 0 : 0.5)
            )
        )
    );
}

bool Player::deactivate(int x, int y, int dist) {
    // A player can never be deactivated
    return false;
}

unsigned Player::getLives() {
    return lives;
}

void Player::endHurtTransition(std::shared_ptr<AnimationInstance> animation) {
    controllable = true;
}

void Player::endHPoleTransition(std::shared_ptr<AnimationInstance> animation) {
    --poleSpinCount;
    if (poleSpinCount > 0) {
        setTransition(AnimState::TRANSITION_POLE_H, false, true, false, &Player::endHPoleTransition);
    } else {
        int mp = poleSpinDirectionPositive ? 1 : -1;
        speedX = 10 * mp;
        externalForceX = 10 * mp;
        controllable = true;
        isGravityAffected = true;
        isFacingLeft = !poleSpinDirectionPositive;
    }
}

void Player::endVPoleTransition(std::shared_ptr<AnimationInstance> animation) {
    --poleSpinCount;
    if (poleSpinCount > 0) {
        setTransition(AnimState::TRANSITION_POLE_V, false, true, false, &Player::endVPoleTransition);
    } else {
        int mp = poleSpinDirectionPositive ? 1 : -1;
        posY += mp * 32;
        speedY = 10 * mp;
        externalForceY = -1 * mp;
        controllable = true;
        isGravityAffected = true;
    }
}

void Player::endWarpTransition(std::shared_ptr<AnimationInstance> animation) {
    auto events = root->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    if (transition->getAnimationState() == AnimState::TRANSITION_WARP) {
        quint16 p[8];
        events->getPositionParams(posX, posY, p);
        CoordinatePair c = events->getWarpTarget(p[0]);
        moveInstantly(c); // validity checked when warping started
        setTransition(AnimState::TRANSITION_WARP_END, false, true, false, &Player::endWarpTransition);
        playSound("COMMON_WARP_OUT");
    } else {
        isInvulnerable = false;
        isGravityAffected = true;
        controllable = true;
    }
}

LevelCarryOver Player::prepareLevelCarryOver() {
    LevelCarryOver o;
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

void Player::receiveLevelCarryOver(LevelCarryOver o) {
    lives = o.lives;
    fastfires = o.fastfires;
    for (int i = 0; i < WEAPONCOUNT; ++i) {
        ammo[i] = o.ammo[i];
        isWeaponPoweredUp[i] = o.poweredUp[i];
    }
    score = o.score;
    foodCounter = o.foodCounter;
    currentWeapon = o.currentWeapon;
}

void Player::addScore(unsigned points) {
    score = std::min(99999999ul, score + points);
    osd->setScore(score);
}

void Player::setCarryingPlatform(std::weak_ptr<MovingPlatform> platform) {
    if (speedY < -1e-6) {
        return;
    }

    carryingObject = platform;
    canJump = true;
    internalForceY = 0;
    speedY = 0;
}

void Player::setView(std::shared_ptr<GameView> view) {
    assignedView = view;
}

void Player::setupOSD(OSDMessageType type, int param) {
    switch (type) {
        case OSD_GEM_RED: 
            osd->setMessage(OSD_GEM_RED, collectedGems[0] + 5 * collectedGems[1] + 10 * collectedGems[2]);
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
            osd->setMessage(OSD_COIN_SILVER, collectedCoins[0] + 5 * collectedCoins[1]);
            break;
        case OSD_COIN_GOLD: 
            osd->setMessage(OSD_COIN_GOLD, collectedCoins[1]);
            break;
        case OSD_BONUS_WARP_NOT_ENOUGH_COINS: 
            osd->setMessage(OSD_BONUS_WARP_NOT_ENOUGH_COINS, param);
            break;
    }
}

template<typename T> std::shared_ptr<T> Player::fireWeapon() {
    auto weakPtr = std::dynamic_pointer_cast<Player>(shared_from_this());
    bool lookup = ((currentAnimation->getAnimationState() & AnimState::LOOKUP) > 0);
    auto animation = currentAnimation->getAnimation();
    int fire_x = (animation->hotspot.x - animation->gunspot.x) * (isFacingLeft ? 1 : -1);
    int fire_y =  animation->hotspot.y - animation->gunspot.y;

    auto newAmmo = std::make_shared<T>(root, weakPtr, posX + fire_x, posY - fire_y, speedX, isFacingLeft, lookup);
    root->addActor(newAmmo);
    return newAmmo;
}

const uint Player::SUGAR_RUSH_THRESHOLD = 100;