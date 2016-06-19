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
#include "../struct/Constants.h"
#include "MovingPlatform.h"
#include "collectible/GemCollectible.h"
#include "collectible/CoinCollectible.h"

enum PlayerCharacter {
    CHAR_JAZZ       = 0x00,
    CHAR_SPAZ       = 0x01,
    CHAR_LORI       = 0x02,
    CHAR_FROG       = 0x80,
    CHAR_BIRD       = 0x81
};

struct LevelCarryOver {
    uint lives;
    uint ammo[WEAPONCOUNT];
    bool poweredUp[WEAPONCOUNT];
    uint fastfires;
    uint score;
    uint foodCounter;
    WeaponType currentWeapon;
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
    void addAmmo(WeaponType type, unsigned amount);
    void addGems(GemType type, unsigned amount);
    void addCoins(CoinType type, unsigned amount);
    void addFastFire(unsigned amount);
    void addHealth(unsigned amount);
    void consumeFood(const bool& isDrinkable);
    void setCarryingPlatform(std::weak_ptr<MovingPlatform> platform);
    void setView(std::shared_ptr<GameView> view);

#ifdef CARROT_DEBUG
public slots:
    void debugHealth();
    void debugAmmo();
#endif

private:
    void onHitFloorHook();
    void onHitCeilingHook();
    void onHitWallHook();
    bool selectWeapon(enum WeaponType newType);
    void setupOSD(OSDMessageType type, int param = 0);
    template<typename T> std::shared_ptr<T> fireWeapon();
    uint getGemsTotalValue();
    uint getCoinsTotalValue();

    PlayerCharacter character;
    std::unique_ptr<PlayerOSD> osd;
    ControlScheme controls;
    std::shared_ptr<GameView> assignedView;

    std::weak_ptr<MovingPlatform> carryingObject;

    unsigned lives;
    unsigned ammo[WEAPONCOUNT];
    bool isWeaponPoweredUp[WEAPONCOUNT];
    uint fastfires;
    unsigned long score;
    unsigned collectedGems[4];
    unsigned collectedCoins[2];
    unsigned foodCounter;

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

    bool isSugarRush;
    static const uint SUGAR_RUSH_THRESHOLD;

    CoordinatePair warpTarget;

private slots:
    void delayedUppercutStart(std::shared_ptr<AnimationInstance> animation);
    void delayedButtstompStart(std::shared_ptr<AnimationInstance> animation);
    void endDamagingMove();
    void returnControl();
};
