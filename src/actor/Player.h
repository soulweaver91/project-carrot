#pragma once

#include <memory>
#include <SFML/Graphics.hpp>

#include "InteractiveActor.h"
#include "../gamestate/PlayerOSD.h"
#include "../graphics/BitmapFont.h"
#include "../struct/Controls.h"
#include "../struct/WeaponTypes.h"
#include "../struct/Constants.h"
#include "../struct/NextLevelData.h"
#include "MovingPlatform.h"
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

class Player : public QObject, public InteractiveActor {

    Q_OBJECT

public:
    Player(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0);
    ~Player();
    void processControlDownEvent(const ControlEvent& e) override;
    void processControlUpEvent(const ControlEvent& e) override;
    void processControlHeldEvent(const ControlEvent& e) override;
    void processAllControlHeldEvents(const QMap<Control, ControlState>& e) override;
    void tickEvent();
    void drawUIOverlay();
    unsigned getHealth();
    unsigned getLives();
    bool getPowerUp(WeaponType type) const;
    bool perish();
    Hitbox getHitbox() override;
    bool setPlayerTransition(AnimStateT state, bool cancellable, bool removeControl = false, 
        bool setSpecial = false, AnimationCallbackFunc callback = []() {});
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
    template<typename T> std::shared_ptr<T> fireWeapon(bool poweredUp);
    uint getGemsTotalValue();
    uint getCoinsTotalValue();
    void warpToPosition(const CoordinatePair& pos);
    void endDamagingMove();

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

    bool isUsingDamagingMove;
    bool isAttachedToPole;
    int cameraShiftFramesCount;
    int copterFramesLeft;

    // Counter for Toaster ammo subticks
    unsigned short toasterAmmoSubticks;

    void initialPoleStage(bool horizontal);
    void nextPoleStage(bool horizontal, bool positive, ushort stagesLeft);

    bool isSugarRush;
    static const uint SUGAR_RUSH_THRESHOLD;
};
