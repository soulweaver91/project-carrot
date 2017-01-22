#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

#include "InteractiveActor.h"
#include "lighting/RadialLightSource.h"
#include "../gamestate/PlayerOSD.h"
#include "../struct/Controls.h"
#include "../struct/WeaponTypes.h"
#include "../struct/Constants.h"
#include "../struct/NextLevelData.h"
#include "solidobj/MovingPlatform.h"
#include "collectible/GemCollectible.h"
#include "collectible/CoinCollectible.h"

class ActorAPI;

enum PlayerCharacter {
    CHAR_JAZZ       = 0x00,
    CHAR_SPAZ       = 0x01,
    CHAR_LORI       = 0x02,
    CHAR_FROG       = 0x80,
    CHAR_BIRD       = 0x81
};

enum SpecialMoveType {
    SPECIAL_MOVE_NONE,
    SPECIAL_MOVE_BUTTSTOMP,
    SPECIAL_MOVE_UPPERCUT,
    SPECIAL_MOVE_SIDEKICK
};

class Player : public QObject, public InteractiveActor, public RadialLightSource {

    Q_OBJECT

public:
    Player(const ActorInstantiationDetails& initData);
    ~Player();
    void processControlDownEvent(const ControlEvent& e) override;
    void processControlUpEvent(const ControlEvent& e) override;
    void processControlHeldEvent(const ControlEvent& e) override;
    void processAllControlHeldEvents(const QMap<Control, ControlState>& e) override;
    void tickEvent() override;
    void drawUIOverlay();
    unsigned getHealth();
    unsigned getLives();
    bool getPowerUp(WeaponType type) const;
    bool perish() override;
    void updateHitbox() override;
    bool setPlayerTransition(AnimStateT state, bool cancellable, bool removeControl = false, 
        SpecialMoveType setSpecial = SPECIAL_MOVE_NONE, AnimationCallbackFunc callback = []() {});
    void takeDamage(double pushForce);
    void setToOwnViewCenter();
    bool deactivate(int x, int y, int dist) override;
    LevelCarryOver prepareLevelCarryOver();
    void receiveLevelCarryOver(LevelCarryOver o);
    void addScore(unsigned points);
    void addAmmo(WeaponType type, unsigned amount);
    void addGems(GemType type, unsigned amount);
    void addCoins(CoinType type, unsigned amount);
    void addFastFire(unsigned amount);
    void addHealth(unsigned amount);
    void setPowerUp(WeaponType type);
    void consumeFood(const bool& isDrinkable);
    void setCarryingPlatform(std::weak_ptr<MovingPlatform> platform);
    void setView(std::shared_ptr<GameView> view);
    void setExiting(ExitType e);

#ifdef CARROT_DEBUG
public slots:
    void debugHealth();
    void debugAmmo();
#endif

private:
    void onHitFloorHook() override;
    void onHitCeilingHook() override;
    void onHitWallHook() override;
    bool selectWeapon(enum WeaponType newType);
    void setupOSD(OSDMessageType type, int param = 0);
    template<typename T> std::shared_ptr<T> fireWeapon(bool poweredUp);
    uint getGemsTotalValue();
    uint getCoinsTotalValue();
    void warpToPosition(const CoordinatePair& pos);
    void endDamagingMove();
    void verifyOSDInitialized();
    void followCarryingPlatform();
    void updateSpeedBasedAnimation(double lastX);
    void updateCameraPosition();
    void pushSolidObjects();
    void checkEndOfSpecialMoves();
    void checkDestructibleTiles();
    void checkSuspendedStatus();
    void handleAreaEvents();
    void handleActorCollisions();

    PlayerCharacter character;
    std::unique_ptr<PlayerOSD> osd;
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

    SpecialMoveType currentSpecialMove;
    bool isAttachedToPole;
    bool isActivelyPushing;
    int cameraShiftFramesCount;
    int copterFramesLeft;
    bool levelExiting;

    // Counter for Toaster ammo subticks
    unsigned short toasterAmmoSubticks;

    void initialPoleStage(bool horizontal);
    void nextPoleStage(bool horizontal, bool positive, ushort stagesLeft);

    bool isSugarRush;
    static const uint SUGAR_RUSH_THRESHOLD;
    static const double MAX_DASHING_SPEED;
    static const double MAX_RUNNING_SPEED;
    static const double ACCELERATION;
    static const double DECELERATION;
};
