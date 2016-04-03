#pragma once

#include <memory>

#include "../CommonActor.h"
#include "../Player.h"

class CarrotQt5;

class Ammo : public CommonActor {
public:
    Ammo(std::shared_ptr<CarrotQt5> root, std::weak_ptr<Player> firedBy = std::weak_ptr<Player>(),
        double x = 0.0, double y = 0.0, bool firedLeft = false, bool firedUp = false, int alive = 70, bool powered = false);
    ~Ammo();
    void tickEvent();

protected:
    void checkCollisions();
    bool powered_up;
    virtual void ricochet();
    
private:
    std::weak_ptr<Player> owner;
    double start_x;
    double start_y;
    int ttl;
};
