#include <memory>
#include "Ammo.h"
#include "CarrotQt5.h"
#include "CommonActor.h"
#include "TileMap.h"
#include "EventMap.h"
#include "Enemy.h"
#include "Collectible.h"
#include "PushBox.h"

Ammo::Ammo(std::shared_ptr<CarrotQt5> root, Player* firedBy, double x, double y, bool firedLeft, bool firedUp, int alive, bool powered)
    : CommonActor(root, x, y, false), owner(firedBy), start_x(x), start_y(y), ttl(alive), powered_up(powered) {
    facingLeft = firedLeft;
}

Ammo::~Ammo() {

}

void Ammo::tickEvent() {
    // Do not use the common actor movement handler here by default
    // Each ammo type needs to implement additional movement code separately
    checkCollisions();
    ttl--;
    if (ttl <= 0) {
        health = 0;
    }
}

void Ammo::checkCollisions() {
    QList< CommonActor* > collisions = root->findCollisionActors(getHitbox(), this);
    for (unsigned i = 0; i < collisions.size(); ++i) {
        // Different things happen with different actor types
        Enemy* enemy = dynamic_cast< Enemy* >(collisions.at(i));
        if (enemy != nullptr) {
            enemy->decreaseHealth(1);
            decreaseHealth(1);
            return;
        }

        Collectible* coll = dynamic_cast< Collectible* >(collisions.at(i));
        if (coll != nullptr) {
            coll->impact(speed_h / 5.0,-speed_v / 10.0);
        }

        PushBox* box = dynamic_cast< PushBox* >(collisions.at(i));
        if (box != nullptr) {
            decreaseHealth(1);
        }

    }
    
    PCEvent e = root->game_events->getPositionEvent(pos_x,pos_y);
    switch(e) {
        case PC_MODIFIER_RICOCHET:
            ricochet();
    }

    if (root->game_tiles->checkWeaponDestructible(pos_x, pos_y)) {
        health = 0;
    }
}

void Ammo::ricochet() {
    speed_v = speed_v * -0.9 + (qrand() % 100 - 50.0) / 80;
    speed_h = speed_h * -0.9 + (qrand() % 100 - 50.0) / 80;
}
