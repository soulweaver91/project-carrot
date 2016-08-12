#pragma once

#include <memory>

#include "../CommonActor.h"
#include "../Player.h"

class CarrotQt5;

class Ammo : public CommonActor {
public:
    Ammo(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double x = 0.0, double y = 0.0, bool firedLeft = false, bool firedUp = false, int lifeLength = 70, bool powered = false);
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
