#pragma once

#include <memory>
#include <QKeyEvent>
#include <SFML/Graphics.hpp>

#include "../CarrotQt5.h"
#include "CommonActor.h"
#include "../gamestate/PlayerOSD.h"
#include "../graphics/BitmapFont.h"
#include "../struct/Controls.h"
#include "../struct/WeaponTypes.h"
#include "MovingPlatform.h"

enum PlayerCharacter {
    CHAR_JAZZ       = 0x00,
    CHAR_SPAZ       = 0x01,
    CHAR_LORI       = 0x02,
    CHAR_FROG       = 0x80,
    CHAR_BIRD       = 0x81
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
    void processControlDownEvent(const ControlEvent& e) override;
    void processControlUpEvent(const ControlEvent& e) override;
    void processControlHeldEvent(const ControlEvent& e) override;
    void processAllControlHeldEvents(const QMap<Control, ControlState>& e) override;
    void tickEvent();
    void drawUIOverlay();
    unsigned getHealth();
    unsigned getLives();
    bool perish();
    Hitbox getHitbox() override;
    bool setTransition(AnimStateT state, bool cancellable, bool removeControl = false, 
        bool setSpecial = false, void(Player::*callback)(std::shared_ptr<AnimationInstance> animation) = nullptr);
    void takeDamage(double pushForce);
    void setToViewCenter();
    bool deactivate(int x, int y, int dist) override;
    LevelCarryOver prepareLevelCarryOver();
    void receiveLevelCarryOver(LevelCarryOver o);
    void addScore(unsigned points);
    void setCarryingPlatform(std::weak_ptr<MovingPlatform> platform);
    void setView(std::shared_ptr<GameView> view);

public slots:
    void debugHealth();
    void debugAmmo();

private:
    void onHitFloorHook();
    void onHitCeilingHook();
    void onHitWallHook();
    bool selectWeapon(enum WeaponType newType);
    void addAmmo(enum WeaponType type, unsigned amount);
    void setupOSD(OSDMessageType type, int param = 0);
    template<typename T> std::shared_ptr<T> fireWeapon();

    PlayerCharacter character;
    std::unique_ptr<PlayerOSD> osd;
    ControlScheme controls;
    std::shared_ptr<GameView> assignedView;

    std::weak_ptr<MovingPlatform> carryingObject;

    unsigned lives;
    unsigned ammo[9];
    bool isWeaponPoweredUp[9];
    int fastfires;
    unsigned long score;
    unsigned collectedGems[4];
    unsigned collectedCoins[2];

    enum WeaponType currentWeapon;
    unsigned weaponCooldown;

    bool isUsingDamagingMove;
    bool controllable;
    int cameraShiftFramesCount;
    int copterFramesLeft;

    // Variables for spinning poles: counter to have three loops, boolean value to know which direction the pole is supposed to move you to
    unsigned short poleSpinCount;
    bool poleSpinDirectionPositive;

    // Counter for Toaster ammo subticks
    unsigned short toasterAmmoSubticks;

    void endHurtTransition(std::shared_ptr<AnimationInstance> animation);
    void endHPoleTransition(std::shared_ptr<AnimationInstance> animation);
    void endVPoleTransition(std::shared_ptr<AnimationInstance> animation);
    void endWarpTransition(std::shared_ptr<AnimationInstance> animation);
    void deathRecovery(std::shared_ptr<AnimationInstance> animation);

private slots:
    void delayedUppercutStart(std::shared_ptr<AnimationInstance> animation);
    void delayedButtstompStart(std::shared_ptr<AnimationInstance> animation);
    void endDamagingMove();
    void returnControl();
};
