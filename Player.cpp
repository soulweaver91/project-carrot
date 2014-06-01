#include "Player.h"

#include "CarrotQt5.h"
#include "SolidObject.h"
#include "TriggerCrate.h"
#include "CommonActor.h"
#include "Enemy.h"
#include "Collectible.h"
#include "TileMap.h"
#include "EventMap.h"
#include "BitmapFont.h"
#include "WeaponTypes.h"
#include "SavePoint.h"

#include <AmmoClasses/AmmoBlaster.h>
#include <AmmoClasses/AmmoBouncer.h>
#include <AmmoClasses/AmmoToaster.h>

#include <SFML/Graphics.hpp>
#include <QTimer>

Player::Player(CarrotQt5* root, double x, double y) : CommonActor(root, x, y, false), weapon_cooldown(0), character(CHAR_JAZZ),
    controllable(true), damaging_move(false), gem_sfx_idx(0), gem_sfx_idx_ctr(0), camera_shift(0), copter_time(0), fastfires(0),
    transition_end_function(nullptr), pole_spins(0), pole_positive(false), toaster_ammo_ticks(10), score(0), osd_timer(-1l),
    osd_type(OSD_NONE) {
    unsigned anim_idx;
    max_health = 5;
    health = 5;
    lives = 3;
    livesString = new BitmapString(root->mainFont,"x3");
    currentWeapon = WEAPON_BLASTER;
    std::fill_n(ammo,9,0);
    std::fill_n(collected_coins,2,0);
    std::fill_n(collected_gems,4,0);

    addAnimation(AnimState::IDLE,                       "Data/Assets/jazz_stand.png",        1,1,34,48,1 ,16,47);
    addAnimation(AnimState::RUN,                        "Data/Assets/jazz_run.png",          8,1,56,43,15,24,42);
    addAnimation(AnimState::JUMP,                       "Data/Assets/jazz_jump.png",         3,1,40,49,15,23,47);
    addAnimation(AnimState::RUN | AnimState::JUMP,      "Data/Assets/jazz_jump_diag.png",    9,1,49,55,15,21,42);
    addAnimation(AnimState::FALL,                       "Data/Assets/jazz_fall.png",         3,1,57,47,15,25,43);
    addAnimation(AnimState::RUN | AnimState::FALL,      "Data/Assets/jazz_fall_diag.png",    3,1,34,47,30,15,44);
    addAnimation(AnimState::DASH,                       "Data/Assets/jazz_dash.png",         4,1,67,40,15,30,33);
    addAnimation(AnimState::DASH | AnimState::JUMP,     "Data/Assets/jazz_jump_dash.png",    8,1,35,28,15,19,29);
    addAnimation(AnimState::DASH | AnimState::FALL,     "Data/Assets/jazz_fall_diag.png",    3,1,34,47,15,15,38);
    addAnimation(AnimState::LOOKUP,                     "Data/Assets/jazz_lookup.png",       1,1,48,38,1 ,22,37);
    addAnimation(AnimState::CROUCH,                     "Data/Assets/jazz_crouch.png",       1,1,56,20,1 ,17,19);
    addAnimation(AnimState::SHOOT,                      "Data/Assets/jazz_shoot.png",        2,1,44,43,8,17,42);
    addAnimation(AnimState::CROUCH | AnimState::SHOOT,  "Data/Assets/jazz_crouch_shoot.png", 1,2,63,21,8,16,20);
    addAnimation(AnimState::LOOKUP | AnimState::SHOOT,  "Data/Assets/jazz_shoot_up.png",     2,1,37,66,8,19,64);
    addAnimation(AnimState::HURT,                       "Data/Assets/jazz_hurt.png",         9,1,72,64,15,33,52);
    addAnimation(AnimState::UPPERCUT,                   "Data/Assets/jazz_uppercut.png",     3,1,23,55,11,11,36);
    addAnimation(AnimState::BUTTSTOMP,                  "Data/Assets/jazz_buttstomp.png",    8,1,47,58,15,23,56);
    addAnimation(AnimState::HOOK,                       "Data/Assets/jazz_vine.png",         1,1,30,50,1 , 9,34);
    addAnimation(AnimState::HOOK | AnimState::RUN,      "Data/Assets/jazz_vine_move.png",    8,1,55,65,15,27,44);

    // temp
    anim_idx = addAnimation(AnimState::COPTER,                     "Data/Assets/jazz_copter.png",        8,1,36,52,10,18,47);
    assignAnimation(AnimState::COPTER | AnimState::FALL, anim_idx);
    assignAnimation(AnimState::COPTER | AnimState::FALL | AnimState::WALK, anim_idx);
    assignAnimation(AnimState::COPTER | AnimState::FALL | AnimState::RUN, anim_idx);
    assignAnimation(AnimState::COPTER | AnimState::FALL | AnimState::DASH, anim_idx);

    anim_idx = addAnimation(AnimState::SHOOT | AnimState::FALL, "Data/Assets/jazz_fall_shoot.png",2,1,43,56,20,11,50);
    assignAnimation(AnimState::JUMP | AnimState::SHOOT, anim_idx);
    assignAnimation(AnimState::WALK | AnimState::FALL | AnimState::SHOOT, anim_idx);
    assignAnimation(AnimState::WALK | AnimState::JUMP | AnimState::SHOOT, anim_idx);
    assignAnimation(AnimState::RUN  | AnimState::FALL | AnimState::SHOOT, anim_idx);
    assignAnimation(AnimState::RUN  | AnimState::JUMP | AnimState::SHOOT, anim_idx);
    assignAnimation(AnimState::DASH | AnimState::FALL | AnimState::SHOOT, anim_idx);
    assignAnimation(AnimState::DASH | AnimState::JUMP | AnimState::SHOOT, anim_idx);
    
    addAnimation(AnimState::TRANSITION_RUN_TO_IDLE,       "Data/Assets/jazz_run_to_stand.png",      8,1,73,56,15,32,54);
    addAnimation(AnimState::TRANSITION_RUN_TO_DASH,       "Data/Assets/jazz_run_to_dash.png",       8,1,67,35,20,30,34);
    addAnimation(AnimState::TRANSITION_IDLE_FALL_TO_IDLE, "Data/Assets/jazz_fall_to_stand.png",     5,1,62,46,15,20,45);
    addAnimation(AnimState::TRANSITION_IDLE_SHOOT_TO_IDLE,"Data/Assets/jazz_shoot_to_stand.png",    4,1,49,56,15,20,54);
    addAnimation(AnimState::TRANSITION_UPPERCUT_A,        "Data/Assets/jazz_crouch_to_uppercut.png",4,1,55,52,15,17,46);
    addAnimation(AnimState::TRANSITION_UPPERCUT_B,        "Data/Assets/jazz_uppercut_start.png"    ,3,1,55,52,15,17,46);
    addAnimation(AnimState::TRANSITION_END_UPPERCUT,      "Data/Assets/jazz_uppercut_to_jump.png"  ,2,1,50,51,15,31,44);
    addAnimation(AnimState::TRANSITION_BUTTSTOMP_START,   "Data/Assets/jazz_buttstomp_start.png"   ,8,1,47,52,15,24,44);
    addAnimation(AnimState::TRANSITION_POLE_H,            "Data/Assets/jazz_hpole.png"             ,1,6,100,31,24,49,30);
    addAnimation(AnimState::TRANSITION_POLE_H_SLOW,       "Data/Assets/jazz_hpole.png"             ,1,6,100,31,12,49,30);
    addAnimation(AnimState::TRANSITION_POLE_V,            "Data/Assets/jazz_vpole.png"             ,6,1,32,101,24,16,52);
    addAnimation(AnimState::TRANSITION_POLE_V_SLOW,       "Data/Assets/jazz_vpole.png"             ,6,1,32,101,12,16,52);
    addAnimation(AnimState::TRANSITION_DEATH,             "Data/Assets/jazz_die.png"               ,5,4,98,76,15,55,64);
    addAnimation(AnimState::TRANSITION_WARP,              "Data/Assets/jazz_warp_in.png"           ,7,1,29,73,15,16,57);
    addAnimation(AnimState::TRANSITION_WARP_END,          "Data/Assets/jazz_warp_out.png"          ,4,2,58,73,15,16,57);

    anim_idx = addAnimation(AnimState::UI_PLAYER_FACE, "Data/Assets/ui_player_jazz.png",  37,1,36,38,10,0,0);
    ui_icon_sprite = new sf::Sprite();
    ui_icon_sprite->setTexture(*(animation_bank.at(anim_idx)->animation_frames));
    ui_icon_sprite->setTextureRect(sf::IntRect(0,0,animation_bank.at(anim_idx)->frame_width,animation_bank.at(anim_idx)->frame_height));
    ui_icon_sprite->setPosition(5,root->getViewHeight() - 40);

    addTimer(7u,true,static_cast< ActorFunc >(&Player::advanceCharIconFrame));

    heart_tex = sf::Texture();
    heart_tex.loadFromFile("Data/Assets/heart.png");
    
    std::fill_n(weapon_ui_animations, 9, -1);
    weapon_ui_animations[0] = addAnimation(AnimState::UI_WEAPON_BLASTER, "Data/Assets/ui_weapon_blaster.png",  10,1,17,22,10,5,12);
    weapon_ui_animations[1] = addAnimation(AnimState::UI_WEAPON_BOUNCER, "Data/Assets/ui_weapon_bouncer.png",  10,1,16,13,10,5,7);
    weapon_ui_animations[2] = addAnimation(AnimState::UI_WEAPON_FREEZER, "Data/Assets/ui_weapon_freezer.png",  10,1,13,15,10,4,8);
    weapon_ui_animations[3] = addAnimation(AnimState::UI_WEAPON_SEEKER,  "Data/Assets/ui_weapon_seeker.png",   10,1,19,20,10,7,10);
    weapon_ui_animations[4] = addAnimation(AnimState::UI_WEAPON_RF,      "Data/Assets/ui_weapon_rf.png",       10,1,13,20,10,4,10);
    weapon_ui_animations[5] = addAnimation(AnimState::UI_WEAPON_TOASTER, "Data/Assets/ui_weapon_toaster.png",  10,1,16,14,10,5,7);
    weapon_ui_animations[6] = addAnimation(AnimState::UI_WEAPON_TNT,     "Data/Assets/ui_weapon_tnt.png",      10,1,20,27,10,7,13);
    weapon_ui_animations[7] = addAnimation(AnimState::UI_WEAPON_PEPPER,  "Data/Assets/ui_weapon_pepper.png",   9 ,1,15,13,10,5,7);
    weapon_ui_animations[8] = addAnimation(AnimState::UI_WEAPON_ELECTRO, "Data/Assets/ui_weapon_electro.png",  10,1,30,21,10,14,10);
    ui_weapon_sprite = new sf::Sprite();
    ui_weapon_sprite->setTexture(*(animation_bank.at(weapon_ui_animations[0])->animation_frames));
    ui_weapon_sprite->setTextureRect(sf::IntRect(0,0,animation_bank.at(weapon_ui_animations[0])->frame_width,animation_bank.at(weapon_ui_animations[0])->frame_height));
    ui_weapon_sprite->setPosition(root->getViewWidth()  - 85 - animation_bank.at(weapon_ui_animations[0])->offset_x,
                                  root->getViewHeight() - 15 - animation_bank.at(weapon_ui_animations[0])->offset_y);
    
    addAnimation(AnimState::UI_OSD_GEM_RED, "Data/Assets/gem_red.png",  8,1,25,26,10,10,13);
    addAnimation(AnimState::UI_OSD_GEM_GREEN, "Data/Assets/gem_green.png",  8,1,25,26,10,10,13);
    addAnimation(AnimState::UI_OSD_GEM_BLUE, "Data/Assets/gem_blue.png",  8,1,25,26,10,10,13);

    setAnimation(AnimState::FALL);
    
    // Define the score string resource
    scoreString = new BitmapString(root->mainFont,"00000000",FONT_ALIGN_LEFT);
    
    
    osd_string = new BitmapString(root->mainFont,"",FONT_ALIGN_CENTER);

    // Get a brief invincibility at the start of the level
    isInvulnerable = true;
    isBlinking = true;
    addTimer(210u,false,static_cast< ActorFunc >(&Player::removeInvulnerability));
}

Player::~Player() {
    delete livesString;
    delete scoreString;
}

void Player::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
        case Qt::Key_Left:
            facingLeft = true;
            if (controllable) {
                setAnimation(current_animation->state & ~(AnimState::LOOKUP | AnimState::CROUCH));
            }
            break;
        case Qt::Key_Right:
            facingLeft = false;
            if (controllable) {
                setAnimation(current_animation->state & ~(AnimState::LOOKUP | AnimState::CROUCH));
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
            root->sfxsys->playSFX(SFX_SWITCH_AMMO);
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
                    if ((current_animation->state & AnimState::CROUCH) > 0) {
                        controllable = false;
                        setAnimation(AnimState::UPPERCUT);
                        setTransition(AnimState::TRANSITION_UPPERCUT_A,true,true,true,&Player::delayedUppercutStart);
                    } else {
                        if (speed_v > 0 && !canJump) {
                            isGravityAffected = false;
                            speed_v = 1.5;
                            if ((current_animation->state & AnimState::COPTER) == 0) {
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
                setAnimation(current_animation->state & ~AnimState::LOOKUP);
                break;
            case Qt::Key_Space:
                setAnimation(current_animation->state & ~AnimState::SHOOT);
                weapon_cooldown = 0;
                break;
            case Qt::Key_Down:
                setAnimation(current_animation->state & ~AnimState::CROUCH);
                break;
        }
    }
}

void Player::tickEvent() {
    // Check for pushing
    if (canJump && controllable) {
        SolidObject* object;
        if (!(root->isPositionEmpty(CarrotQt5::calcHitbox(getHitbox(),speed_h,0),false,this,object))) {
            if (object != nullptr) {
                object->push(speed_h < 0);
                setAnimation(current_animation->state | AnimState::PUSH);
            } else {
                setAnimation(current_animation->state & ~AnimState::PUSH);
            }
        } else {
            setAnimation(current_animation->state & ~AnimState::PUSH);
        }
    }

    CommonActor::tickEvent();
    short sign = ((speed_h + push) > 1e-6) ? 1 : (((speed_h + push) < -1e-6) ? -1 : 0);
    double gravity = (isGravityAffected ? root->gravity : 0);


    // Check if hitting a vine
    if (root->game_tiles->isPosVine(pos_x,pos_y - 25)) {
        isSuspended = true;
        isGravityAffected = false;
        speed_v = 0;
        thrust = 0;

        // move downwards until we're on the standard height
        while (root->game_tiles->isPosVine(pos_x,pos_y - 25)) {
            pos_y -= 1;
        }
        pos_y += 5;
    } else {
        isSuspended = false;
        if (((current_animation->state & (AnimState::BUTTSTOMP | AnimState::COPTER)) == 0) && (pole_spins == 0)) {
            isGravityAffected = true;
        }
    }

    // Buttstomp/etc. tiles checking
    if ((current_animation->state & (AnimState::BUTTSTOMP | AnimState::UPPERCUT | AnimState::SIDEKICK)) > 0) {
        // check all corners of hitbox
        root->game_tiles->checkSpecialDestructible(pos_x - 10 + speed_h, pos_y - 30 + speed_v);
        root->game_tiles->checkSpecialDestructible(pos_x + 10 + speed_h, pos_y - 30 + speed_v);
        root->game_tiles->checkSpecialDestructible(pos_x - 10 + speed_h, pos_y + speed_v);
        root->game_tiles->checkSpecialDestructible(pos_x + 10 + speed_h, pos_y + speed_v);
        
        SolidObject* obj;
        if (!(root->isPositionEmpty(CarrotQt5::calcHitbox(getHitbox(),speed_h,speed_v),false,this,obj))) {
            TriggerCrate* trcrate = dynamic_cast< TriggerCrate* >(obj);
            if (trcrate != nullptr) {
                trcrate->decreaseHealth(1);
            }
        }
    }

    // check if buttstomp ended
    if (canJump && (current_animation->state & AnimState::BUTTSTOMP) > 0 || isSuspended) {
        setAnimation(current_animation->state & ~AnimState::BUTTSTOMP);
        damaging_move = false;
        controllable = true;
    }

    // check if copter ears ended
    if ((current_animation->state & (AnimState::COPTER)) > 0) {
        if (canJump || copter_time == 0) {
            isGravityAffected = true;
            setAnimation(current_animation->state & ~AnimState::COPTER);
        } else {
            if (copter_time > 0) {
                copter_time--;
            }
        }
    }

    // check if uppercut ended
    if (((current_animation->state & (AnimState::UPPERCUT)) > 0) && speed_v > -2 && !canJump) {
        endDamagingMove();
    }
    
    PCEvent e = root->game_events->getPositionEvent(pos_x,pos_y-15);
    quint16 p[8];
    root->game_events->getPositionParams(pos_x,pos_y-15,p);
    switch (e) {
        case PC_LIGHT_SET:
            root->setLighting(p[0],false);
            break;
        case PC_WARP_ORIGIN:
            {
                if (!inTransition || cancellableTransition) {
                    CoordinatePair c = root->game_events->getWarpTarget(p[0]);
                    if (c.x >= 0) {
                        setTransition(AnimState::TRANSITION_WARP,false,true,false,&Player::endWarpTransition);
                        isInvulnerable = true;
                        isGravityAffected = false;
                        speed_h = 0;
                        speed_v = 0;
                        push = 0;
                        thrust = 0;
                        root->sfxsys->playSFX(SFX_WARP_IN);
                    }
                }
            }
            break;
        case PC_MODIFIER_H_POLE:
            if (pole_spins == 0) {
                pos_y = (qRound(pos_y - 15) / 32) * 32 + 30;
                setTransition(AnimState::TRANSITION_POLE_H_SLOW,false,true,false,&Player::endHPoleTransition);
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
                pos_y = (qRound(pos_y - 15) / 32) * 32 + 15;
                setTransition(AnimState::TRANSITION_POLE_V_SLOW,false,true,false,&Player::endVPoleTransition);
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
                root->setMusic("");
                root->sfxsys->playSFX(SFX_JAZZ_EOL);
                root->initLevelChange(NEXT_NORMAL);
            }
            controllable = false;
            break;
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
    if ((current_animation->state & AnimState::CROUCH) > 0) {
        // Down is being held, move camera one unit down
        camera_shift = std::min(128,camera_shift + 1);
    } else if ((current_animation->state & AnimState::LOOKUP) > 0) {
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
            speed_h = std::max((abs(speed_h) - 0.25),0.0) * (facingLeft ? -1 : 1);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            setAnimation(current_animation->state | AnimState::SHOOT);
            bool crouch = ((current_animation->state & AnimState::CROUCH) > 0);
            bool lookup = ((current_animation->state & AnimState::LOOKUP) > 0);
            int fire_x = (lookup ? -4 : 23) * (facingLeft ? -1 : 1);
            int fire_y = (lookup ? 0 : -5) + (crouch ? 18 : 31);
            if (weapon_cooldown == 0) {
                switch (currentWeapon) {
                    case WEAPON_BLASTER:
                        {
                            Ammo_Blaster* newAmmo = new Ammo_Blaster(root,this,pos_x + fire_x,pos_y - fire_y,facingLeft,lookup);
                            root->addActor(newAmmo);
                            weapon_cooldown = std::max(0,40 - 3 * fastfires);
                            root->sfxsys->playSFX(SFX_BLASTER_SHOOT_JAZZ);
                            break;
                        }
                    case WEAPON_BOUNCER:
                        {
                            Ammo_Bouncer* newAmmo = new Ammo_Bouncer(root,this,pos_x + fire_x,pos_y - fire_y,facingLeft,lookup);
                            root->addActor(newAmmo);
                            weapon_cooldown = 25;
                            break;
                        }
                    case WEAPON_TOASTER:
                        {
                            Ammo_Toaster* newAmmo = new Ammo_Toaster(root,this,pos_x + fire_x,pos_y - fire_y,facingLeft,lookup);
                            root->addActor(newAmmo);
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
                            root->sfxsys->playSFX(SFX_TOASTER_SHOOT);
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
                pos_y -= 10;
                canJump = true;
            }
            if (canJump && ((current_animation->state & AnimState::UPPERCUT) == 0) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                thrust = 1.2;
                speed_v = -3 - std::max(0.0, (std::abs(speed_h) - 4.0) * 0.3);
                canJump = false;
                setAnimation(current_animation->state & (~AnimState::LOOKUP & ~AnimState::CROUCH));
                root->sfxsys->playSFX(SFX_JUMP);
            }
        } else {
            if (thrust > 0) {
                thrust = 0;
            }
        }
    }

    QList< CommonActor* > collisions = root->findCollisionActors(getHitbox(), this);
    for (unsigned i = 0; i < collisions.size(); ++i) {
        // Different things happen with different actor types

        Enemy* enemy = dynamic_cast< Enemy* >(collisions.at(i));
        if (enemy != nullptr) {
            if (damaging_move) {
                enemy->decreaseHealth(1);
                if ((current_animation->state & AnimState::BUTTSTOMP) > 0) {
                    setAnimation(current_animation->state & ~AnimState::BUTTSTOMP);
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
        
        SavePoint* sp = dynamic_cast< SavePoint* >(collisions.at(i));
        if (sp != nullptr) {
            sp->activateSavePoint();
        }

        Collectible* coll = dynamic_cast< Collectible* >(collisions.at(i));
        if (coll != nullptr) {
            switch(coll->type) {
                case COLLTYPE_FAST_FIRE:
                    fastfires = std::min(fastfires + 1, 10);
                    root->sfxsys->playSFX(SFX_COLLECT_AMMO);
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_TOASTER:
                    addAmmo(WEAPON_TOASTER, 3);
                    root->sfxsys->playSFX(SFX_COLLECT_AMMO);
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_SEEKER:
                    addAmmo(WEAPON_SEEKER, 3);
                    root->sfxsys->playSFX(SFX_COLLECT_AMMO);
                    addScore(100);
                    break;
                case COLLTYPE_AMMO_BOUNCER:
                    addAmmo(WEAPON_BOUNCER, 3);
                    root->sfxsys->playSFX(SFX_COLLECT_AMMO);
                    addScore(100);
                    break;
                case COLLTYPE_GEM_RED:
                    addScore(100);
                    root->sfxsys->playSFX(SFX_COLLECT_GEM,gem_sfx_idx);
                    gem_sfx_idx = (gem_sfx_idx + 1) % 6;
                    gem_sfx_idx_ctr = 180;
                    collected_gems[0]++;
                    setupOSD(OSD_GEM_RED);
                    break;
                case COLLTYPE_GEM_GREEN:
                    addScore(500);
                    root->sfxsys->playSFX(SFX_COLLECT_GEM,gem_sfx_idx);
                    gem_sfx_idx = (gem_sfx_idx + 1) % 6;
                    gem_sfx_idx_ctr = 180;
                    collected_gems[1]++;
                    setupOSD(OSD_GEM_GREEN);
                    break;
                case COLLTYPE_GEM_BLUE:
                    addScore(1000);
                    root->sfxsys->playSFX(SFX_COLLECT_GEM,gem_sfx_idx);
                    gem_sfx_idx = (gem_sfx_idx + 1) % 6;
                    gem_sfx_idx_ctr = 180;
                    collected_gems[2]++;
                    setupOSD(OSD_GEM_BLUE);
                    break;
                case COLLTYPE_COIN_GOLD:
                    addScore(1000);
                    root->sfxsys->playSFX(SFX_COLLECT_COIN);
                    collected_coins[1]++;
                    setupOSD(OSD_COIN_GOLD);
                    break;
                case COLLTYPE_COIN_SILVER:
                    addScore(500);
                    root->sfxsys->playSFX(SFX_COLLECT_COIN);
                    collected_coins[0]++;
                    setupOSD(OSD_COIN_SILVER);
                    break;
            }
            collisions.at(i)->deleteFromEventMap();
            delete collisions.at(i);
        }
    }
}

void Player::advanceCharIconFrame() {
    ui_icon_frame = (ui_icon_frame + 1) % 37;
    ui_icon_sprite->setTextureRect(sf::IntRect(ui_icon_frame*36,0,36,38));

    if (weapon_ui_animations[currentWeapon] != -1) {
        ui_weapon_frame = (ui_weapon_frame + 1) % animation_bank.at(weapon_ui_animations[currentWeapon])->frame_cols;
    
        ui_weapon_sprite->setTextureRect(sf::IntRect(
            ui_weapon_frame*animation_bank.at(weapon_ui_animations[currentWeapon])->frame_width,0,
            animation_bank.at(weapon_ui_animations[currentWeapon])->frame_width,
            animation_bank.at(weapon_ui_animations[currentWeapon])->frame_height));
    }

}

unsigned Player::getHealth() {
    return health;
}

void Player::drawUIOverlay() {
    unsigned vw = root->getViewWidth();
    unsigned vh = root->getViewHeight();
    root->window->draw(*ui_icon_sprite);
    root->window->draw(*ui_weapon_sprite);
    livesString->drawString(root->window,40,vh - 25);

    sf::Sprite heartspr(heart_tex);
    for (unsigned i = 0; i < health; ++i) {
        heartspr.setPosition((vw - 100.0) + i * 18.0, 5.0);
        if (!((health == 1) && ((root->getFrame() % 6) > 2))) {
            root->window->draw(heartspr);
        }
    }

    BitmapString ammo_str(root->mainFont,"x?");
    if (currentWeapon == WEAPON_BLASTER) {
        ammo_str.setText("x^");
    } else {
        ammo_str.setText(QString("x") + QString::number(ammo[currentWeapon]));
    }
    ammo_str.drawString(root->window,vw - 70,vh - 25);
    
    BitmapString::drawString(root->window,root->mainFont,"P1: " + QString::number(pos_x) + "," + QString::number(pos_y),6,86);
    BitmapString::drawString(root->window,root->mainFont,"  Hsp " + QString::number(speed_h),6,116);
    BitmapString::drawString(root->window,root->mainFont,"  Vsp " + QString::number(speed_v),6,146);
    
    // Draw the current score
    scoreString->setText(QString::number(score).rightJustified(8,'0',false));
    scoreString->drawString(root->window,6,6);

    if (osd_type != OSD_NONE) {
        osd_offset = std::min(60u,osd_offset + 1);
    } else {
        if (osd_offset > 0) {
            osd_offset--;
        }
    }
    if (osd_offset > 0) {
        osd_string->drawString(root->window,vw / 2 + 30 - osd_offset / 2,vh - osd_offset / 2);
        switch (osd_type) {
            case OSD_GEM_RED:

            case OSD_NONE:
            default:
                break;
        }
    }
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

    // Otherwise all OK; let's check to be sure that the weapon icon is loaded
    // If it is, set that to be the animated one; otherwise just let it show the
    // earlier weapon
    if (weapon_ui_animations[new_type] != -1) {
        StateAnimationPair* wp = animation_bank.at(weapon_ui_animations[new_type]);
        ui_weapon_sprite->setTexture(*(wp->animation_frames));
        ui_weapon_sprite->setTextureRect(sf::IntRect(0,0,wp->frame_width,wp->frame_height));
        ui_weapon_sprite->setPosition(root->getViewWidth() - 85 - wp->offset_x, root->getViewHeight() - 15 - wp->offset_y);
        ui_weapon_frame = 0;
    }

    currentWeapon = new_type;
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
        health = 5;
        livesString->setText("x" + QString::number(lives));

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
    Hitbox nbox = {pos_x - 10, pos_y - 30, pos_x + 10, pos_y};
    return nbox;
}

void Player::endDamagingMove() {
    damaging_move = false;
    controllable = true;
    setAnimation(current_animation->state & ~AnimState::UPPERCUT & ~AnimState::SIDEKICK & ~AnimState::BUTTSTOMP);
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

bool Player::setTransition(ActorState state, bool cancellable, bool remove_control, bool set_special, void(Player::*callback)()) {
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
    if (root->game_events->isPosHurting(pos_x,pos_y)) {
        takeDamage(speed_h / 4);
    } else {
        if (!canJump) {
            root->sfxsys->playSFX(SFX_LAND);
        }
    }
}

void Player::onHitCeilingHook() {
    if (root->game_events->isPosHurting(pos_x,pos_y - 32)) {
        takeDamage(speed_h / 4);
    }
}

void Player::onHitWallHook() {
    if (root->game_events->isPosHurting(pos_x + (speed_h > 0 ? 1 : -1) * 16,pos_y-16)) {
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
        addTimer(210u,false,static_cast< ActorFunc >(&Player::removeInvulnerability));
        root->sfxsys->playSFX(SFX_JAZZ_HURT);
    }
}

void Player::setToViewCenter(sf::View* view) {
    int shift_offset = 0;
    if (abs(camera_shift) > 48) {
        shift_offset = (abs(camera_shift) - 48) * (camera_shift > 0 ? 1 : -1);
    }
    view->setCenter(
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
    if (transition->state == AnimState::TRANSITION_WARP) {
        quint16 p[8];
        root->game_events->getPositionParams(pos_x,pos_y-15,p);
        CoordinatePair c = root->game_events->getWarpTarget(p[0]);
        moveInstantly(c); // validity checked when warping started
        setTransition(AnimState::TRANSITION_WARP_END,false,true,false,&Player::endWarpTransition);
        root->sfxsys->playSFX(SFX_WARP_OUT);
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

    // Update the lives string as it doesn't do so automatically
    livesString->setText("x" + QString::number(lives));
}

void Player::addScore(unsigned points) {
    score = std::min(99999999ul,score + points);
}

void Player::setupOSD(OSDType type, int param) {
    cancelTimer(osd_timer);
    osd_offset = 0;
    osd_type = type;
    osd_timer = addTimer(350u,false,static_cast< ActorFunc >(&Player::clearOSD));
    switch (type) {
        case OSD_GEM_RED: 
            osd_string->setText("  x" + QString::number(collected_gems[0] + 5 * collected_gems[1] + 10 * collected_gems[2]));
            break;
        case OSD_GEM_GREEN: 
            osd_string->setText("  x" + QString::number(collected_gems[1]));
            break;
        case OSD_GEM_BLUE: 
            osd_string->setText("  x" + QString::number(collected_gems[2]));
            break;
        case OSD_COIN_SILVER: 
            osd_string->setText("  x" + QString::number(collected_coins[0] + 5 * collected_coins[1]));
            break;
        case OSD_COIN_GOLD: 
            osd_string->setText("  x" + QString::number(collected_coins[1]));
            break;
        case OSD_BONUS_WARP_NOT_ENOUGH_COINS: 
            osd_string->setText("need   x" + QString::number(param) + " more");
            break;
    }
}

void Player::clearOSD() {
    osd_type = OSD_NONE;
}
