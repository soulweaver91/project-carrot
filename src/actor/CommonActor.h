#pragma once

#include <memory>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "../CarrotQt5.h"
#include "../gamestate/TileMap.h"

class CommonActor;

typedef unsigned ActorState;
typedef void(CommonActor::*ActorFunc)();

namespace AnimState {
    // Bits 0, 1: horizontal speed (none, low, med, high)
    unsigned const IDLE         = 0x00000000;
    unsigned const WALK         = 0x00000001;
    unsigned const RUN          = 0x00000002;
    unsigned const DASH         = 0x00000003;

    // Bits 2, 3: vertical speed (none, upwards, downwards, suspended)
    unsigned const V_IDLE       = 0x00000000;
    unsigned const JUMP         = 0x00000004;
    unsigned const FALL         = 0x00000008;
    unsigned const HOOK         = 0x0000000c;

    // Bit 4: shoot
    unsigned const SHOOT        = 0x00000010;

    // Bits 5-9: multiple special stances that cannot occur together
    // but still have unique bits due to complications in determining
    // the current actor state
    unsigned const CROUCH       = 0x00000020;
    unsigned const LOOKUP       = 0x00000040;
    unsigned const IDLEANIM     = 0x00000080;
    unsigned const BUTTSTOMP    = 0x00000100;
    unsigned const UPPERCUT     = 0x00000200;
    unsigned const SIDEKICK     = 0x00000400;
    unsigned const HURT         = 0x00000800;
    unsigned const SWIM         = 0x00001000;
    unsigned const COPTER       = 0x00002000;
    unsigned const PUSH         = 0x00004000;
    unsigned const SWINGROPE    = 0x00008000;
    
    // 30th bit: transition range
    unsigned const TRANSITION_RUN_TO_IDLE                   = 0x40000000;
    unsigned const TRANSITION_RUN_TO_DASH                   = 0x40000001;
    unsigned const TRANSITION_IDLE_FALL_TO_IDLE             = 0x40000002;
    unsigned const TRANSITION_IDLE_TO_IDLE_JUMP             = 0x40000003;
    unsigned const TRANSITION_IDLE_SHOOT_TO_IDLE            = 0x40000004;
    unsigned const TRANSITION_IDLE_FALL_SHOOT_TO_IDLE_SHOOT = 0x40000005;
    unsigned const TRANSITION_IDLE_SHOOT_TO_IDLE_JUMP_SHOOT = 0x40000006;
    unsigned const TRANSITION_UPPERCUT_A                    = 0x40000007;
    unsigned const TRANSITION_UPPERCUT_B                    = 0x40000008;
    unsigned const TRANSITION_END_UPPERCUT                  = 0x40000009;
    unsigned const TRANSITION_BUTTSTOMP_START               = 0x4000000A;
    unsigned const TRANSITION_POLE_H                        = 0x4000000B;
    unsigned const TRANSITION_POLE_V                        = 0x4000000C;
    unsigned const TRANSITION_POLE_H_SLOW                   = 0x4000000D;
    unsigned const TRANSITION_POLE_V_SLOW                   = 0x4000000E;
    unsigned const TRANSITION_DEATH                         = 0x4000000F;
    unsigned const TRANSITION_TURN                          = 0x40000010;
    unsigned const TRANSITION_WITHDRAW                      = 0x40000011;
    unsigned const TRANSITION_WITHDRAW_END                  = 0x40000012;
    unsigned const TRANSITION_WARP                          = 0x40000013;
    unsigned const TRANSITION_WARP_END                      = 0x40000014;

    // 31st bit: UI icon range
    unsigned const UI_PLAYER_FACE       = 0x80000000;
    unsigned const UI_WEAPON_BLASTER    = 0x80000001;
    unsigned const UI_WEAPON_BOUNCER    = 0x80000002;
    unsigned const UI_WEAPON_FREEZER    = 0x80000003;
    unsigned const UI_WEAPON_SEEKER     = 0x80000004;
    unsigned const UI_WEAPON_RF         = 0x80000005;
    unsigned const UI_WEAPON_TOASTER    = 0x80000006;
    unsigned const UI_WEAPON_TNT        = 0x80000007;
    unsigned const UI_WEAPON_PEPPER     = 0x80000008;
    unsigned const UI_WEAPON_ELECTRO    = 0x80000009;
    unsigned const UI_OSD_GEM_RED       = 0x8000000A;
    unsigned const UI_OSD_GEM_GREEN     = 0x8000000B;
    unsigned const UI_OSD_GEM_BLUE      = 0x8000000C;
    unsigned const UI_OSD_GEM_PURPLE    = 0x8000000D;
    unsigned const UI_OSD_COIN_GOLD     = 0x8000000E;
    unsigned const UI_OSD_COIN_SILVER   = 0x8000000F;
    unsigned const UI_OSD_STOPWATCH     = 0x80000010;
    
    // Aliases for object states overlapping player states
    unsigned const ACTIVATED            = 0x00000020;
    unsigned const TRANSITION_ACTIVATE  = 0x4F000000;

    unsigned const STATE_UNINITIALIZED  = 0xFFFFFFFF;
}

struct StateAnimationPair {
    ActorState state;
    sf::Texture* animation_frames;
    int frame_width;
    int frame_height;
    int frame_rows;
    int frame_cols;
    int offset_x;
    int offset_y;
    int frametime;
};

struct ActorTimer {
    // current state
    unsigned long frames_left;
    double frames_remainder; // increased by frames_original_remainder by every call if recurring
    // initial state
    unsigned long frames_original;
    double frames_original_remainder;
    bool recurring;

    // callback
    ActorFunc func;
};

class CommonActor : public QObject, public std::enable_shared_from_this<CommonActor> {
public:
    CommonActor(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0, bool fromEventMap = false);
    ~CommonActor();
    size_t addAnimation(ActorState state, const QString& filename, 
        int frame_cols = 1, int frame_rows = 1, int frame_width = -1,
        int frame_height = -1, int fps = 30, int offset_x = 0, int offset_y = 0);
    bool setAnimation(ActorState state);
    virtual bool setTransition(ActorState state, bool cancellable);
    virtual void DrawUpdate();
    virtual void tickEvent();
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    void decreaseHealth(unsigned amount = 1);
    virtual void setToViewCenter(sf::View* view);
    CoordinatePair getPosition();
    virtual Hitbox getHitbox();
    virtual bool perish();
    virtual bool deactivate(int x, int y, int dist);
    void moveInstantly(CoordinatePair location);
    void deleteFromEventMap();
    void advanceTimers();
        
protected:
    size_t assignAnimation(ActorState state, size_t original_idx);
    bool setAnimation(StateAnimationPair* animation);
    void animationAdvance();
    void removeInvulnerability();
    virtual void onHitFloorHook();
    virtual void onHitCeilingHook();
    virtual void onHitWallHook();
    virtual void onTransitionEndHook();
    std::shared_ptr<CarrotQt5> root;
    sf::RenderWindow* destWindow;
    unsigned max_health;
    unsigned health;
    double pos_x;
    double pos_y;
    double speed_h;
    double speed_v;
    double thrust;
    double push;
    unsigned frame;
    sf::Sprite sprite;
    StateAnimationPair* current_animation;
    QList< StateAnimationPair* > animation_bank;
    bool inTransition;
    bool cancellableTransition;
    StateAnimationPair* transition;
    bool canJump;
    bool facingLeft;
    bool isGravityAffected;
    bool isClippingAffected;
    double elasticity;
    double friction;
    bool isInvulnerable;
    bool isBlinking;
    bool isSuspended;
    bool createdFromEventMap;
    int origin_x;
    int origin_y;

    // timers
    virtual unsigned long addTimer(double frames, bool recurring, ActorFunc func);
    virtual unsigned long addTimer(unsigned frames, bool recurring, ActorFunc func);
    virtual void invokeTimer(int idx);
    void cancelTimer(unsigned long idx);
    QList< QPair< unsigned long, ActorTimer > > timers;
    unsigned long next_timer;
    unsigned long animation_timer;
};
