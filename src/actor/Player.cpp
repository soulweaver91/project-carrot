#include "Player.h"
#include "../gamestate/TileMap.h"
#include "../gamestate/EventMap.h"
#include "SolidObject.h"
#include "TriggerCrate.h"
#include "enemy/Enemy.h"
#include "Collectible.h"
#include "SavePoint.h"
#include "Spring.h"
#include "weapon/AmmoBlaster.h"
#include "weapon/AmmoBouncer.h"
#include "weapon/AmmoToaster.h"

Player::Player(std::shared_ptr<CarrotQt5> root, double x, double y) : CommonActor(root, x, y, false), weapon_cooldown(0), character(CHAR_JAZZ),
    controllable(true), damaging_move(false), gem_sfx_idx(0), gem_sfx_idx_ctr(0), camera_shift(0), copter_time(0), fastfires(0),
    transition_end_function(nullptr), pole_spins(0), pole_positive(false), toaster_ammo_ticks(10), score(0) {
    unsigned anim_idx;
    max_health = 5;
    health = 5;
    lives = 3;
    currentWeapon = WEAPON_BLASTER;
    std::fill_n(ammo,9,0);
    std::fill_n(collected_coins,2,0);
    std::fill_n(collected_gems,4,0);

    loadResources("Interactive/PlayerJazz");
    setAnimation(AnimState::FALL);

    // Get a brief invincibility at the start of the level
    isInvulnerable = true;
    isBlinking = true;
    addTimer(210u,false,static_cast<TimerCallbackFunc>(&Player::removeInvulnerability));
}

Player::~Player() {

}

void Player::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
        case Qt::Key_Left:
            facingLeft = true;
            if (controllable) {
                setAnimation(currentState & ~(AnimState::LOOKUP | AnimState::CROUCH));
            }
            break;
        case Qt::Key_Right:
            facingLeft = false;
            if (controllable) {
                setAnimation(currentState & ~(AnimState::LOOKUP | AnimState::CROUCH));
            }
            break;
        case Qt::Key_Up:
            if (controllable && canJump && std::abs(speed_h < 1e-6)) {
                setAnimation(AnimState::LOOKUP);
            }
            break;
        case Qt::Key_Space:
            // only TNT needs to be handled here
            if (canJump) {
            }
            break;
        case Qt::Key_Down:
            if (controllable) {
                if (canJump && std::abs(speed_h < 1e-6)) {
                    setAnimation(AnimState::CROUCH);
                } else {
                    if (isSuspended) {
                        pos_y += 10;
                        isSuspended = false;
                    } else {
                        controllable = false;
                        speed_h = 0;
                        speed_v = 0;
                        thrust = 0;
                        isGravityAffected = false;
                        damaging_move = true;
                        setAnimation(AnimState::BUTTSTOMP);
                        setTransition(AnimState::TRANSITION_BUTTSTOMP_START,true,false,false,&Player::delayedButtstompStart);
                        break;
                    }
                }
            }
            break;
        case Qt::Key_1:
            selectWeapon(WEAPON_BLASTER); break;
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            playSound("PLAYER_SWITCH_AMMO");
            // Key_2 is 50 and the rest come sequentially so no need for a separate
            // case branch for each key this way
            // Change this if the key enum codes change in Qt for any reason
            selectWeapon(static_cast<WeaponType>(event->key() - 49));
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            {
                // Select next available weapon in numerical order
                int new_type = (currentWeapon + 1) % 9;
                while (!selectWeapon(static_cast< WeaponType >(new_type))) {
                    new_type = (new_type + 1) % 9;
                }
            }
            break;
        case Qt::Key_Control:
            switch(character) {
                case CHAR_JAZZ:
                    if ((currentState & AnimState::CROUCH) > 0) {
                        controllable = false;
                        setAnimation(AnimState::UPPERCUT);
                        setTransition(AnimState::TRANSITION_UPPERCUT_A,true,true,true,&Player::delayedUppercutStart);
                    } else {
                        if (speed_v > 0 && !canJump) {
                            isGravityAffected = false;
                            speed_v = 1.5;
                            if ((currentState & AnimState::COPTER) == 0) {
                                setAnimation(AnimState::COPTER);
                            }
                            copter_time = 50;
                        }
                    }
                    break;
                case CHAR_SPAZ:
                case CHAR_LORI:
                    // sidekick
                    break;
            }
            break;
    }
}

void Player::keyReleaseEvent(QKeyEvent* event) {
    if (controllable) {
        switch(event->key()) {
            case Qt::Key_Left:
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                    facingLeft = false;
                }
                break;
            case Qt::Key_Right:
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                    facingLeft = true;
                }
                break;
            case Qt::Key_Up:
                setAnimation(currentState & ~AnimState::LOOKUP);
                break;
            case Qt::Key_Space:
                setAnimation(currentState & ~AnimState::SHOOT);
                weapon_cooldown = 0;
                break;
            case Qt::Key_Down:
                setAnimation(currentState & ~AnimState::CROUCH);
                break;
        }
    }
}

void Player::tickEvent() {
    // Initialize these ASAP
    if (osd == nullptr) {
        osd = std::make_unique<PlayerOSD>(root, std::dynamic_pointer_cast<Player>(shared_from_this()), root->getCanvas());
        osd->setWeaponType(currentWeapon, ammo_powered[currentWeapon]);
        osd->setAmmo(ammo[currentWeapon]);
        osd->setLives(lives);
        osd->setScore(score);
        osd->setHealth(health);
    }

    auto tiles = root->getGameTiles().lock();

    // Check for pushing
    if (canJump && controllable) {
        std::weak_ptr<SolidObject> object;
        if (!(root->isPositionEmpty(CarrotQt5::calcHitbox(getHitbox(), speed_h, 0), false, shared_from_this(), object))) {
            auto objectPtr = object.lock();

            if (objectPtr != nullptr) {
                objectPtr->push(speed_h < 0);
                setAnimation(currentState | AnimState::PUSH);
            } else {
                setAnimation(currentState & ~AnimState::PUSH);
            }
        } else {
            setAnimation(currentState & ~AnimState::PUSH);
        }
    }

    CommonActor::tickEvent();
    short sign = ((speed_h + push) > 1e-6) ? 1 : (((speed_h + push) < -1e-6) ? -1 : 0);
    double gravity = (isGravityAffected ? root->gravity : 0);


    // Check if hitting a vine
    if (tiles != nullptr && tiles->isPosVine(pos_x,pos_y - 5)) {
        isSuspended = true;
        isGravityAffected = false;
        speed_v = 0;
        thrust = 0;

        // move downwards until we're on the standard height
        while (tiles->isPosVine(pos_x,pos_y - 5)) {
            pos_y += 1;
        }
        pos_y -= 1;
    } else {
        isSuspended = false;
        if (((currentState & (AnimState::BUTTSTOMP | AnimState::COPTER)) == 0) && (pole_spins == 0)) {
            isGravityAffected = true;
        }
    }

    // Buttstomp/etc. tiles checking
    if (tiles != nullptr && (currentState & (AnimState::BUTTSTOMP | AnimState::UPPERCUT | AnimState::SIDEKICK)) > 0) {
        // check all corners of hitbox
        tiles->checkSpecialDestructible(pos_x - 14 + speed_h, pos_y - 6 + speed_v);
        tiles->checkSpecialDestructible(pos_x + 14 + speed_h, pos_y - 6 + speed_v);
        tiles->checkSpecialDestructible(pos_x - 14 + speed_h, pos_y + 22 + speed_v);
        tiles->checkSpecialDestructible(pos_x + 14 + speed_h, pos_y + 22 + speed_v);

        std::weak_ptr<SolidObject> object;
        if (!(root->isPositionEmpty(CarrotQt5::calcHitbox(getHitbox(), speed_h, speed_v), false, shared_from_this(), object))) {
            auto trcrate = std::dynamic_pointer_cast<TriggerCrate>(object.lock());
            if (trcrate != nullptr) {
                trcrate->decreaseHealth(1);
            }
        }
    }

    // check if buttstomp ended
    if (canJump && (currentState & AnimState::BUTTSTOMP) > 0 || isSuspended) {
        setAnimation(currentState & ~AnimState::BUTTSTOMP);
        damaging_move = false;
        controllable = true;
    }

    // check if copter ears ended
    if ((currentState & (AnimState::COPTER)) > 0) {
        if (canJump || copter_time == 0) {
            isGravityAffected = true;
            setAnimation(currentState & ~AnimState::COPTER);
        } else {
            if (copter_time > 0) {
                copter_time--;
            }
        }
    }

    // check if uppercut ended
    if (((currentState & (AnimState::UPPERCUT)) > 0) && speed_v > -2 && !canJump) {
        endDamagingMove();
    }
    
    auto events = root->getGameEvents().lock();
    if (events != nullptr) {
        PCEvent e = events->getPositionEvent(pos_x, pos_y);
        quint16 p[8];
        events->getPositionParams(pos_x, pos_y, p);
        switch (e) {
            case PC_LIGHT_SET:
                root->setLighting(p[0], false);
                break;
            case PC_WARP_ORIGIN:
            {
                if (!inTransition || cancellableTransition) {
                    CoordinatePair c = events->getWarpTarget(p[0]);
                    if (c.x >= 0) {
                        setTransition(AnimState::TRANSITION_WARP, false, true, false, &Player::endWarpTransition);
                        isInvulnerable = true;
                        isGravityAffected = false;
                        speed_h = 0;
                        speed_v = 0;
                        push = 0;
                        thrust = 0;
                        playSound("COMMON_WARP_IN");
                    }
                }
            }
            break;
            case PC_MODIFIER_H_POLE:
                if (pole_spins == 0) {
                    pos_y = (qRound(pos_y - 15) / 32) * 32 + 16;
                    setTransition(AnimState::TRANSITION_POLE_H_SLOW, false, true, false, &Player::endHPoleTransition);
                    pole_positive = (speed_h > 0);
                    pos_x = (qRound(pos_x) / 32) * 32 + 16;
                    pole_spins = 3;
                    speed_h = 0;
                    speed_v = 0;
                    push = 0;
                    thrust = 0;
                    canJump = false;
                    isGravityAffected = false;
                }
                break;
            case PC_MODIFIER_V_POLE:
                if (pole_spins == 0) {
                    pos_y = (qRound(pos_y) / 32) * 32 + 16;
                    setTransition(AnimState::TRANSITION_POLE_V_SLOW, false, true, false, &Player::endVPoleTransition);
                    pole_positive = (speed_v > 0);
                    pos_x = (qRound(pos_x) / 32) * 32 + 16;
                    pole_spins = 3;
                    speed_h = 0;
                    speed_v = 0;
                    push = 0;
                    thrust = 0;
                    canJump = false;
                    isGravityAffected = false;
                }
                break;
            case PC_AREA_EOL:
                if (controllable) {
                    playSound("PLAYER_JAZZ_EOL");
                    root->initLevelChange(NEXT_NORMAL);
                }
                controllable = false;
                break;
        }
    }
    
    // reduce player timers for certain things:
    // Weapon cooldown
    if (weapon_cooldown > 0) {
        weapon_cooldown--;
    }

    // Gem sound counter (we want to go from lowest to highest pitch, but reset to lowest if
    // no gems are collected for a while)
    // TODO: delete and replace with a HUD version
    if (gem_sfx_idx_ctr > 0) {
        gem_sfx_idx_ctr--;

        if (gem_sfx_idx_ctr == 0) {
            gem_sfx_idx = 0;
        }
    }
    
    // Move camera if up or down held
    if ((currentState & AnimState::CROUCH) > 0) {
        // Down is being held, move camera one unit down
        camera_shift = std::min(128,camera_shift + 1);
    } else if ((currentState & AnimState::LOOKUP) > 0) {
        // Up is being held, move camera one unit up
        camera_shift = std::max(-128,camera_shift - 1);
    } else {
        // Neither is being held, move camera up to 10 units back to equilibrium
        camera_shift = (camera_shift > 0 ? 1 : -1) * std::max(0, abs(camera_shift) - 10);
    }

    if (controllable) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            if (!isSuspended && (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)
                || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift))) {
                speed_h = std::max(std::min(speed_h + 0.2 * (facingLeft ? -1 : 1),9.0),-9.0);
            } else {
                speed_h = std::max(std::min(speed_h + 0.2 * (facingLeft ? -1 : 1),3.0),-3.0);
            }
        } else {
            speed_h = std::max((abs(speed_h) - 0.25),0.0) * (speed_h < -1e-6 ? -1 : 1);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            setAnimation(currentState | AnimState::SHOOT);
            if (weapon_cooldown == 0) {
                switch (currentWeapon) {
                    case WEAPON_BLASTER:
                        {
                            auto newAmmo = fireWeapon<Ammo_Blaster>();
                            weapon_cooldown = std::max(0, 40 - 3 * fastfires);
                            playSound("WEAPON_BLASTER_JAZZ");
                            break;
                        }
                    case WEAPON_BOUNCER:
                        {
                            auto newAmmo = fireWeapon<Ammo_Bouncer>();
                            weapon_cooldown = 25;
                            break;
                        }
                    case WEAPON_TOASTER:
                        {
                            auto newAmmo = fireWeapon<Ammo_Toaster>();
                            weapon_cooldown = 3;
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
                        --toaster_ammo_ticks;
                        if (toaster_ammo_ticks == 0) {
                            ammo[currentWeapon] -= 1;
                            toaster_ammo_ticks = 10;
                        }
                    } else {
                        ammo[currentWeapon] -= 1;
                    }

                    if (ammo[currentWeapon] == 0) {
                        int new_type = (currentWeapon + 1) % 9;
                        // Iterate through weapons to pick the next usable when running out of ammo
                        while (!selectWeapon(static_cast< WeaponType >(new_type))) {
                            new_type = (new_type + 1) % 9;
                        }
                    }
                }
            }
        }

        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))) {
            if (isSuspended) {
                pos_y -= 5;
                canJump = true;
            }
            if (canJump && ((currentState & AnimState::UPPERCUT) == 0) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                thrust = 1.2;
                speed_v = -3 - std::max(0.0, (std::abs(speed_h) - 4.0) * 0.3);
                canJump = false;
                setAnimation(currentState & (~AnimState::LOOKUP & ~AnimState::CROUCH));
                playSound("COMMON_JUMP");
            }
        } else {
            if (thrust > 0) {
                thrust = 0;
            }
        }
    }

    auto collisions = root->findCollisionActors(getHitbox(), shared_from_this());
    foreach (auto collision, collisions) {
        auto collisionPtr = collision.lock();

        // Different things happen with different actor types

        {
            auto enemy = std::dynamic_pointer_cast<Enemy>(collisionPtr);
            if (enemy != nullptr) {
                if (damaging_move) {
                    enemy->decreaseHealth(1);
                    if ((currentState & AnimState::BUTTSTOMP) > 0) {
                        setAnimation(currentState & ~AnimState::BUTTSTOMP);
                        damaging_move = false;
                        controllable = true;
                        speed_v *= -.5;
                    }
                } else {
                    if (enemy->hurtsPlayer()) {
                        takeDamage(4 * (pos_x > enemy->getPosition().x ? 1 : -1));
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
            auto spr = std::dynamic_pointer_cast<Spring>(collisionPtr);
            if (spr != nullptr) {
                sf::Vector2f params = spr->activate();
                short sign = ((params.x + params.y) > 1e-6 ? 1 : -1);
                if (abs(params.x) > 1e-6) {
                    speed_h = (4 + abs(params.x)) * sign;
                    push = params.x;
                    setTransition(AnimState::DASH | AnimState::JUMP, true, false, false);
                } else {
                    speed_v = (4 + abs(params.y / 2)) * sign;
                    thrust = -params.y / 2;
                    setTransition(sign == -1 ? AnimState::TRANSITION_SPRING : AnimState::BUTTSTOMP, true, false, false);
                }
                canJump = false;
                continue;
            }
        }

        auto coll = std::dynamic_pointer_cast<Collectible>(collisionPtr);
        if (coll != nullptr) {
            switch(coll->type) {
                case COLLTYPE_FAST_FIRE:
                    fastfires = std::min(fastfires + 1, 10);
                    playSound("PLAYER_PICKUP_AMMO");
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_TOASTER:
                    addAmmo(WEAPON_TOASTER, 3);
                    playSound("PLAYER_PICKUP_AMMO");
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_SEEKER:
                    addAmmo(WEAPON_SEEKER, 3);
                    playSound("PLAYER_PICKUP_AMMO");
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_BOUNCER:
                    addAmmo(WEAPON_BOUNCER, 3);
                    playSound("PLAYER_PICKUP_AMMO");
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_FREEZER:
                    addAmmo(WEAPON_FREEZER, 3);
                    playSound("PLAYER_PICKUP_AMMO");
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_RF:
                    addAmmo(WEAPON_RF, 3);
                    playSound("PLAYER_PICKUP_AMMO");
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_TNT:
                    addAmmo(WEAPON_TNT, 3);
                    playSound("PLAYER_PICKUP_AMMO");
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_PEPPER:
                    addAmmo(WEAPON_PEPPER, 3);
                    playSound("PLAYER_PICKUP_AMMO");
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_ELECTRO:
                    addAmmo(WEAPON_ELECTRO, 3);
                    playSound("PLAYER_PICKUP_AMMO");
                    addScore(100);
                    break;
                case COLLTYPE_GEM_RED:
                    addScore(100);
                    playSound("PLAYER_PICKUP_GEM");
                    gem_sfx_idx = (gem_sfx_idx + 1) % 6;
                    gem_sfx_idx_ctr = 180;
                    collected_gems[0]++;
                    setupOSD(OSD_GEM_RED);
                    break;
                case COLLTYPE_GEM_GREEN:
                    addScore(500);
                    playSound("PLAYER_PICKUP_GEM");
                    gem_sfx_idx = (gem_sfx_idx + 1) % 6;
                    gem_sfx_idx_ctr = 180;
                    collected_gems[1]++;
                    setupOSD(OSD_GEM_GREEN);
                    break;
                case COLLTYPE_GEM_BLUE:
                    addScore(1000);
                    playSound("PLAYER_PICKUP_GEM");
                    gem_sfx_idx = (gem_sfx_idx + 1) % 6;
                    gem_sfx_idx_ctr = 180;
                    collected_gems[2]++;
                    setupOSD(OSD_GEM_BLUE);
                    break;
                case COLLTYPE_COIN_GOLD:
                    addScore(1000);
                    playSound("PLAYER_PICKUP_COIN");
                    collected_coins[1]++;
                    setupOSD(OSD_COIN_GOLD);
                    break;
                case COLLTYPE_COIN_SILVER:
                    addScore(500);
                    playSound("PLAYER_PICKUP_COIN");
                    collected_coins[0]++;
                    setupOSD(OSD_COIN_SILVER);
                    break;
            }
            collisionPtr->deleteFromEventMap();
            root->removeActor(collisionPtr);
        }
    }
}

unsigned Player::getHealth() {
    return health;
}

void Player::drawUIOverlay() {
    osd->drawOSD();

    auto canvas = root->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    BitmapString::drawString(canvas, root->getFont(), "P1: " + QString::number(pos_x) + "," + QString::number(pos_y), 6, 86);
    BitmapString::drawString(canvas, root->getFont(), "  Hsp " + QString::number(speed_h), 6, 116);
    BitmapString::drawString(canvas, root->getFont(), "  Vsp " + QString::number(speed_v), 6, 146);
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
    osd->setWeaponType(new_type, ammo_powered[new_type]);
    osd->setAmmo(ammo[currentWeapon]);

    return true;
}

void Player::debugHealth() {
    health = 5;
}

void Player::debugAmmo() {
    std::fill_n(ammo,9,999);
}

void Player::addAmmo(enum WeaponType type, unsigned amount) {
    if (type > 8) { return; }
    
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

bool Player::perish() {
    // handle death here
    if (health == 0 && (transition_end_function != &Player::deathRecovery)) {
        cancellableTransition = false;
        setTransition(AnimState::TRANSITION_DEATH,false,true,false,&Player::deathRecovery);
    }
    return false;
}

void Player::deathRecovery() {
    if (lives > 0) {
        lives--;

        // Return us to the last save point
        root->loadSavePoint();

        // Reset health and remove one life
        health = max_health;
        osd->setLives(lives);
        osd->setHealth(max_health);

        // Negate all possible movement effects etc.
        onTransitionEndHook();
        canJump = false;
        push = 0;
        thrust = 0;
        speed_h = 0;
        speed_v = 0;
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
    Hitbox box = {
        pos_x - 12,
        pos_y - 4,
        pos_x + 12,
        pos_y + 20
    };

    return box;
}

void Player::endDamagingMove() {
    damaging_move = false;
    controllable = true;
    setAnimation(currentState & ~AnimState::UPPERCUT & ~AnimState::SIDEKICK & ~AnimState::BUTTSTOMP);
    setTransition(AnimState::TRANSITION_END_UPPERCUT,false);
}

void Player::returnControl() {
    controllable = true;
}

void Player::delayedUppercutStart() {
    thrust = 1.5;
    speed_v = -2;
    canJump = false;
    setTransition(AnimState::TRANSITION_UPPERCUT_B,true,true,true);
}

void Player::delayedButtstompStart() {
    isGravityAffected = true;
    speed_v = 7;
    setAnimation(AnimState::BUTTSTOMP);
}

bool Player::setTransition(AnimStateT state, bool cancellable, bool remove_control, bool set_special, void(Player::*callback)()) {
    transition_end_function = callback;
    bool result = CommonActor::setTransition(state, cancellable);
    if (remove_control) {
        controllable = false;
    }
    if (set_special) {
        damaging_move = true;
    }
    return result;
}

void Player::onHitFloorHook() {
    auto events = root->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    if (events->isPosHurting(pos_x, pos_y + 24)) {
        takeDamage(speed_h / 4);
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

    if (events->isPosHurting(pos_x, pos_y - 4)) {
        takeDamage(speed_h / 4);
    }
}

void Player::onHitWallHook() {
    auto events = root->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    if (events->isPosHurting(pos_x + (speed_h > 0 ? 1 : -1) * 16, pos_y)) {
        takeDamage(speed_h / 4);
    }
}

void Player::takeDamage(double npush) {
    if (!isInvulnerable) {
        health = static_cast<unsigned>(std::max(static_cast<int>(health - 1), 0));
        push = npush;
        thrust = 0;
        speed_v = -6.5;
        speed_h = 0;
        canJump = false;
        setTransition(AnimState::HURT, false, true, false, &Player::endHurtTransition);
        isInvulnerable = true;
        isBlinking = true;
        addTimer(210u,false,static_cast<TimerCallbackFunc>(&Player::removeInvulnerability));
        playSound("PLAYER_JAZZ_HURT");
        osd->setHealth(health);
    }
}

void Player::setToViewCenter() {
    int shift_offset = 0;
    if (abs(camera_shift) > 48) {
        shift_offset = (abs(camera_shift) - 48) * (camera_shift > 0 ? 1 : -1);
    }
    root->centerView(
        std::max(root->getViewWidth() / 2.0,std::min(32.0 * (root->getLevelWidth()+1)  - root->getViewWidth()  / 2.0, (double)qRound(pos_x))),
        std::max(root->getViewHeight()/ 2.0,std::min(32.0 * (root->getLevelHeight()+1) - root->getViewHeight() / 2.0, (double)qRound(pos_y + shift_offset - 15)))
    );
}

bool Player::deactivate(int x, int y, int dist) {
    // A player can never be deactivated
    return false;
}

unsigned Player::getLives() {
    return lives;
}

void Player::onTransitionEndHook() {
    // Execute the defined transition ending function if defined
    if (transition_end_function != nullptr) {
        // Set transition to null before running the function (transition hook may itself invoke a new transition with a callback,
        // but otherwise we want to clear the callback)
        void (Player::*f)() = transition_end_function;
        transition_end_function = nullptr;

        (this->*f)();
    }
}

void Player::endHurtTransition() {
    controllable = true;
}

void Player::endHPoleTransition() {
    --pole_spins;
    if (pole_spins > 0) {
        setTransition(AnimState::TRANSITION_POLE_H,false,true,false,&Player::endHPoleTransition);
    } else {
        int mp = pole_positive ? 1 : -1;
        speed_h = 10 * mp;
        push = 10 * mp;
        controllable = true;
        isGravityAffected = true;
        facingLeft = !pole_positive;
    }
}

void Player::endVPoleTransition() {
    --pole_spins;
    if (pole_spins > 0) {
        setTransition(AnimState::TRANSITION_POLE_V,false,true,false,&Player::endVPoleTransition);
    } else {
        int mp = pole_positive ? 1 : -1;
        pos_y += mp * 32;
        speed_v = 10 * mp;
        thrust = -1 * mp;
        controllable = true;
        isGravityAffected = true;
    }
}

void Player::endWarpTransition() {
    auto events = root->getGameEvents().lock();
    if (events == nullptr) {
        return;
    }

    if (currentTransitionState == AnimState::TRANSITION_WARP) {
        quint16 p[8];
        events->getPositionParams(pos_x, pos_y-15, p);
        CoordinatePair c = events->getWarpTarget(p[0]);
        moveInstantly(c); // validity checked when warping started
        setTransition(AnimState::TRANSITION_WARP_END,false,true,false,&Player::endWarpTransition);
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
    for (int i = 0; i < 9; ++i) {
        o.ammo[i] = ammo[i];
    }
    return o;
}

void Player::receiveLevelCarryOver(LevelCarryOver o) {
    lives = o.lives;
    fastfires = o.fastfires;
    for (int i = 0; i < 9; ++i) {
        ammo[i] = o.ammo[i];
    }
}

void Player::addScore(unsigned points) {
    score = std::min(99999999ul, score + points);
    osd->setScore(score);
}

void Player::setupOSD(OSDMessageType type, int param) {
    switch (type) {
        case OSD_GEM_RED: 
            osd->setMessage(OSD_GEM_RED, collected_gems[0] + 5 * collected_gems[1] + 10 * collected_gems[2]);
            break;
        case OSD_GEM_GREEN:
            osd->setMessage(OSD_GEM_GREEN, collected_gems[1]);
            break;
        case OSD_GEM_BLUE:
            osd->setMessage(OSD_GEM_BLUE, collected_gems[2]);
            break;
        case OSD_COIN_SILVER: 
            osd->setMessage(OSD_COIN_SILVER, collected_coins[0] + 5 * collected_coins[1]);
            break;
        case OSD_COIN_GOLD: 
            osd->setMessage(OSD_COIN_GOLD, collected_coins[1]);
            break;
        case OSD_BONUS_WARP_NOT_ENOUGH_COINS: 
            osd->setMessage(OSD_BONUS_WARP_NOT_ENOUGH_COINS, param);
            break;
    }
}

template<typename T> std::shared_ptr<T> Player::fireWeapon() {
    auto weakPtr = std::dynamic_pointer_cast<Player>(shared_from_this());
    bool crouch = ((currentState & AnimState::CROUCH) > 0);
    bool lookup = ((currentState & AnimState::LOOKUP) > 0);
    int fire_x = (currentAnimation->hotspot.x - currentAnimation->gunspot.x) * (facingLeft ? 1 : -1);
    int fire_y =  currentAnimation->hotspot.y - currentAnimation->gunspot.y;

    auto newAmmo = std::make_shared<T>(root, weakPtr, pos_x + fire_x, pos_y - fire_y, facingLeft, lookup);
    root->addActor(newAmmo);
    osd->setAmmo(ammo[currentWeapon]);
    return newAmmo;
}
