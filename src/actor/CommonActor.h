#pragma once

#include <memory>
#include <QObject>
#include <QKeyEvent>
#include <QBitArray>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "../gamestate/AnimationUser.h"
#include "../gamestate/TimerUser.h"
#include "../struct/Controls.h"
#include "../struct/CoordinatePair.h"
#include "../struct/Hitbox.h"
#include "../struct/Resources.h"
#include "../struct/Layers.h"

class ActorAPI;
class TileMap;
class GameView;
class Ammo;

class CommonActor : public std::enable_shared_from_this<CommonActor>, public AnimationUser {
public:
    CommonActor(std::shared_ptr<ActorAPI> api, double x = 0.0, double y = 0.0, bool fromEventMap = false);
    ~CommonActor();
    virtual void drawUpdate(std::shared_ptr<GameView>& view);
    virtual void tickEvent();
    void decreaseHealth(unsigned amount = 1);
    virtual void setToViewCenter(std::shared_ptr<GameView> view);
    CoordinatePair getPosition();
    virtual void updateHitbox();
    virtual void updateHitbox(const uint& w, const uint& h);
    double getSpeedX();
    double getSpeedY();
    bool getIsCollidable();
    virtual bool perish();
    virtual bool deactivate(int x, int y, int tileDistance);
    virtual void handleCollision(std::shared_ptr<CommonActor> other);
    void moveInstantly(CoordinatePair location, bool absolute);
    void deleteFromEventMap();
    void updateGraphicState();
    void setInvulnerability(uint frames = 210u, bool blink = false);
    void advanceActorAnimationTimers();
    const ActorGraphicState getGraphicState();
    void handleAmmoFrozenStateChange(std::shared_ptr<CommonActor> ammo);
    Hitbox getHitbox();
        
protected:
    bool setAnimation(AnimStateT state) override;
    virtual void onHitFloorHook();
    virtual void onHitCeilingHook();
    virtual void onHitWallHook();
    bool loadResources(const QString& classId);

    template<typename... P>
    bool playSound(const QString& id, P... params);

    template<typename... P>
    bool playNonPositionalSound(const QString& id, P... params);

    std::shared_ptr<ActorAPI> api;
    unsigned maxHealth;
    unsigned health;
    double posX;
    double posY;
    double speedX;
    double speedY;
    double externalForceX;
    double externalForceY;
    double internalForceY;
    bool canJump;
    bool canBeFrozen;
    bool isFacingLeft;
    bool isGravityAffected;
    bool isClippingAffected;
    bool isInvulnerable;
    bool isBlinking;
    bool isCollidable;
    uint frozenFramesLeft;
    double elasticity;
    double friction;
    SuspendType suspendType;
    bool isCreatedFromEventMap;
    int originTileX;
    int originTileY;
    std::shared_ptr<ResourceSet> resources;
    ActorGraphicState currentGraphicState;
    Hitbox currentHitbox;

private:
    template<typename T, typename... P>
    bool callPlaySound(const QString& id, T coordOrBool, P... params);
};
