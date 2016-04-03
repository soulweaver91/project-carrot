#pragma once

#include <memory>
#include <QObject>
#include <QKeyEvent>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "../gamestate/AnimationUser.h"
#include "../gamestate/TimerUser.h"
#include "../sound/SoundSystem.h"
#include "../struct/CoordinatePair.h"
#include "../struct/Hitbox.h"

class CarrotQt5;
class TileMap;

class CommonActor : public QObject, public std::enable_shared_from_this<CommonActor>, public AnimationUser {
public:
    CommonActor(std::shared_ptr<CarrotQt5> root, double x = 0.0, double y = 0.0, bool fromEventMap = false);
    ~CommonActor();
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
        
protected:
    bool setAnimation(ActorState state) override;
    void removeInvulnerability();
    virtual void onHitFloorHook();
    virtual void onHitCeilingHook();
    virtual void onHitWallHook();
    bool playSound(SFXType sound);
    std::shared_ptr<CarrotQt5> root;
    unsigned max_health;
    unsigned health;
    double pos_x;
    double pos_y;
    double speed_h;
    double speed_v;
    double thrust;
    double push;
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
};
