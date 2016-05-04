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

class CarrotQt5;
class TileMap;
class GameView;

class CommonActor : public QObject, public std::enable_shared_from_this<CommonActor>, public AnimationUser {
public:
    CommonActor(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0, bool fromEventMap = false);
    ~CommonActor();
    virtual void drawUpdate(std::shared_ptr<GameView>& view);
    virtual void tickEvent();
    virtual void processControlDownEvent(const ControlEvent& e);
    virtual void processControlUpEvent(const ControlEvent& e);
    virtual void processAllControlHeldEvents(const QMap<Control, ControlState>& e);
    void decreaseHealth(unsigned amount = 1);
    virtual void setToViewCenter(std::shared_ptr<GameView> view);
    CoordinatePair getPosition();
    virtual Hitbox getHitbox();
    virtual Hitbox getHitbox(const uint& w, const uint& h);
    virtual bool perish();
    virtual bool deactivate(int x, int y, int tileDistance);
    void moveInstantly(CoordinatePair location);
    void deleteFromEventMap();
    void updateGraphicState();
    const ActorGraphicState getGraphicState();
        
protected:
    void processAllControlHeldEventsDefaultHandler(const QMap<Control, ControlState>& e);
    virtual void processControlHeldEvent(const ControlEvent& e);
    bool setAnimation(AnimStateT state) override;
    void removeInvulnerability();
    virtual void onHitFloorHook();
    virtual void onHitCeilingHook();
    virtual void onHitWallHook();
    bool loadResources(const QString& classId);

    template<typename... P>
    bool playSound(const QString& id, P... params);

    template<typename... P>
    bool playNonPositionalSound(const QString& id, P... params);

    std::shared_ptr<CarrotQt5> root;
    unsigned maxHealth;
    unsigned health;
    double posX;
    double posY;
    double speedX;
    double speedY;
    double externalForceY;
    double externalForceX;
    double internalForceY;
    bool canJump;
    bool isFacingLeft;
    bool isGravityAffected;
    bool isClippingAffected;
    double elasticity;
    double friction;
    bool isInvulnerable;
    bool isBlinking;
    SuspendType suspendType;
    bool isCreatedFromEventMap;
    int originTileX;
    int originTileY;
    std::shared_ptr<ResourceSet> resources;
    ActorGraphicState currentGraphicState;

private:
    template<typename T, typename... P>
    bool callPlaySound(const QString& id, T coordOrBool, P... params);
};
