#pragma once

#include <memory>

#include "../CommonActor.h"
#include "../Player.h"

class ActorAPI;

class Ammo : public CommonActor {
public:
    Ammo(std::shared_ptr<ActorAPI> api, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double x = 0.0, double y = 0.0, bool firedLeft = false, bool firedUp = false, int lifeLength = 70, bool poweredUp = false);
    ~Ammo();
    void tickEvent();
    std::weak_ptr<Player> getOwner();
    int getStrength();
    virtual WeaponType getType() const;

protected:
    void checkCollisions();
    bool poweredUp;
    virtual void ricochet();
    int strength;
    
private:
    std::weak_ptr<Player> owner;
    int framesLeft;
    bool firedUp;
};
