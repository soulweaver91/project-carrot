#pragma once

#include "CommonActor.h"
#include "CarrotQt5.h"
#include "BitmapFont.h"
#include <SFML/Graphics.hpp>
#include <QTimer>
#include "WeaponTypes.h"

enum PlayerCharacter {
    CHAR_JAZZ       = 0x00,
    CHAR_SPAZ       = 0x01,
    CHAR_LORI       = 0x02,
    CHAR_FROG       = 0x80,
    CHAR_BIRD       = 0x81
};

enum OSDType {
    OSD_NONE,
    OSD_GEM_RED,
    OSD_GEM_GREEN,
    OSD_GEM_BLUE,
    OSD_GEM_PURPLE,
    OSD_COIN_GOLD,
    OSD_COIN_SILVER,
    OSD_BONUS_WARP_NOT_ENOUGH_COINS
};

struct LevelCarryOver {
    int lives;
    int ammo[9];
    int fastfires;
};

class Player : public CommonActor {

    Q_OBJECT

public:
    Player(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0);
    ~Player();
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void tickEvent();
    void drawUIOverlay();
    unsigned getHealth();
    unsigned getLives();
    bool perish();
    Hitbox getHitbox();
    bool setTransition(ActorState state, bool cancellable, bool remove_control = false, bool set_special = false, void(Player::*callback)() = nullptr);
    void takeDamage(double push);
    void setToViewCenter(sf::View* view);
    bool deactivate(int x, int y, int dist) override;
    LevelCarryOver prepareLevelCarryOver();
    void receiveLevelCarryOver(LevelCarryOver o);
    void addScore(unsigned points);

public slots:
    void debugHealth();
    void debugAmmo();

private:
    void onHitFloorHook();
    void onHitCeilingHook();
    void onHitWallHook();
    bool selectWeapon(enum WeaponType new_type);
    void addAmmo(enum WeaponType type, unsigned amount);
    void setupOSD(OSDType type, int param = 0);
    void clearOSD();

    PlayerCharacter character;

    sf::Sprite* ui_icon_sprite;
    unsigned ui_icon_frame;
    sf::Sprite* ui_weapon_sprite;
    unsigned ui_weapon_frame;
    unsigned weapon_ui_animations[9];
    sf::Texture heart_tex;
    BitmapString* livesString;
    BitmapString* scoreString;
    
    OSDType osd_type;
    unsigned long osd_timer;
    unsigned osd_offset;
    BitmapString* osd_string;
    sf::Sprite* osd_sprite;

    unsigned lives;
    unsigned ammo[9];
    bool ammo_powered[9];
    int fastfires;
    unsigned long score;
    unsigned collected_gems[4];
    unsigned collected_coins[4];

    enum WeaponType currentWeapon;
    unsigned weapon_cooldown;

    bool damaging_move;
    bool controllable;
    int camera_shift;
    int gem_sfx_idx;        // to be removed
    int gem_sfx_idx_ctr;
    int copter_time;

    // Variables for spinning poles: counter to have three loops, boolean value to know which direction the pole is supposed to move you to
    unsigned short pole_spins;
    bool pole_positive;

    // Counter for Toaster ammo subticks
    unsigned short toaster_ammo_ticks;

    void onTransitionEndHook();
    void(Player::*transition_end_function)();

    void endHurtTransition();
    void endHPoleTransition();
    void endVPoleTransition();
    void endWarpTransition();
    void deathRecovery();

private slots:
    void advanceCharIconFrame();
    void delayedUppercutStart();
    void delayedButtstompStart();
    void endDamagingMove();
    void returnControl();
};
