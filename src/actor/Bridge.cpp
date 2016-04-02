#include "Bridge.h"
#include "Player.h"

DynamicBridgePiece::DynamicBridgePiece(std::shared_ptr<CarrotQt5> root, double x, double y, DynamicBridgeType type) : SolidObject(root, x, y, false) {
    // temp
    addAnimation(AnimState::IDLE,        "bridge/gem.png",    1,1,14,13,1,0,0);
    setAnimation(AnimState::IDLE);
    
    isGravityAffected = false;
    one_way = true;
}

DynamicBridgePiece::~DynamicBridgePiece() {

}

bool DynamicBridgePiece::deactivate(int x, int y, int dist) {
    // removal of bridge pieces is handled by the bridge event
    return false;
}

Hitbox DynamicBridgePiece::getCollHitbox() {
    Hitbox h = getHitbox();
    h.top -= 2;
    h.bottom += 2;
    return h;
}

DynamicBridge::DynamicBridge(std::shared_ptr<CarrotQt5> root, double x, double y, unsigned int width, DynamicBridgeType type, unsigned int toughness)
    : CommonActor(root, x - 16.0, y - 16.0), toughness(toughness), bridge_width(width), bridge_type(type), original_y(y) {
    for (int i = 0; i < width; ++i) {
        auto piece_n = std::make_shared<DynamicBridgePiece>(root,x + 16 * i - 16,y - 16,type);
        root->addActor(piece_n);
        bridge_objs << piece_n;
    }
    isGravityAffected = false;
    toughness = 7;
}

DynamicBridge::~DynamicBridge() {
    for (int i = 0; i < bridge_objs.size(); ++i) {
        root->removeActor(bridge_objs.at(i));
    }
}

Hitbox DynamicBridge::getHitbox() {
    return CarrotQt5::calcHitbox(pos_x + bridge_width * 8,pos_y - 2,bridge_width * 16,16);
}

void DynamicBridge::tickEvent() {
    // get collision for all bridge elements
    auto collision = root->findCollisionActors(getHitbox(), shared_from_this());
    for (int j = 0; j < bridge_objs.size(); ++j) {
        collision.append(root->findCollisionActors(bridge_objs.at(j)->getCollHitbox(), shared_from_this()));
    }

    bool found = false;
    for (int i = 0; i < collision.size(); ++i) {
        auto collisionPtr = collision.at(i).lock();
        if (collisionPtr == nullptr) {
            continue;
        }

        auto p = std::dynamic_pointer_cast<Player>(collisionPtr);
        if (p != nullptr) {
            found = true;
            CoordinatePair coords = p->getPosition();
            unsigned int num = bridge_objs.size();

            toughness = 0;

            double lowest = (coords.x - pos_x) / 16;
            int drop = std::min((-abs(lowest - num / 2.0) + (num / 2.0)) * 6,2.0 * num - toughness);
            
            pos_y = std::min(original_y + drop, std::max(original_y, coords.y));

            for (int j = 0; j < num; ++j) {
                coords.x = pos_x + 16 * j;

                if (lowest > 0 && lowest < num) {
                    if (j <= lowest) {
                        coords.y = original_y - 2.0 + pow(j / lowest,0.7) * drop;
                    } else {
                        coords.y = original_y - 2.0 + pow((num - j) / (num - lowest),0.7) * drop;
                    }
                } else {
                    coords.y = original_y - 2;
                }

                bridge_objs.at(j)->moveInstantly(coords);
            }
        }

    }

    if (!found) {
        for (int j = 0; j < bridge_objs.size(); ++j) {
            CoordinatePair coords = bridge_objs.at(j)->getPosition();
            coords.y = original_y;
            bridge_objs.at(j)->moveInstantly(coords);
        }
        pos_y = original_y;
    }
}
