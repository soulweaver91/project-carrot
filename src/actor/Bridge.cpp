#include "Bridge.h"
#include "Player.h"

DynamicBridgePiece::DynamicBridgePiece(std::shared_ptr<CarrotQt5> root, double x, double y, DynamicBridgeType type)
    : SolidObject(root, x, y, false), bridgeType(type) {
    loadResources("Object/BridgePiece");
    // temp
    AnimationUser::setAnimation("OBJECT_BRIDGE_PIECE_GEM");
    
    isGravityAffected = false;
    isOneWay = true;
}

DynamicBridgePiece::~DynamicBridgePiece() {

}


bool DynamicBridgePiece::deactivate(int, int, int) {
    // removal of bridge pieces is handled by the bridge event
    return false;
}

Hitbox DynamicBridgePiece::getHitboxForParent() {
    return getHitbox().extend(0, 2);
}

DynamicBridge::DynamicBridge(std::shared_ptr<CarrotQt5> root, double x, double y, unsigned int width,
    DynamicBridgeType type, unsigned int toughness)
    : CommonActor(root, x - 16.0, y - 16.0), originalY(y), toughness(toughness), bridgeType(type), bridgeWidth(width) {
    loadResources("Object/Bridge");

    for (uint i = 0; i < width; ++i) {
        auto piece_n = std::make_shared<DynamicBridgePiece>(root, x + 16 * i - 16, y - 16, type);
        root->addActor(piece_n);
        bridgePieces << piece_n;
    }
    isGravityAffected = false;
    toughness = 7;
}

DynamicBridge::~DynamicBridge() {
    for (int i = 0; i < bridgePieces.size(); ++i) {
        root->removeActor(bridgePieces.at(i));
    }
}

Hitbox DynamicBridge::getHitbox() {
    return Hitbox(posX, posY - 10.0, posX + bridgeWidth * 16.0, posY + 16.0);
}

void DynamicBridge::tickEvent() {
    // get collision for all bridge elements
    auto collision = root->findCollisionActors(getHitbox(), shared_from_this());
    for (int j = 0; j < bridgePieces.size(); ++j) {
        collision.append(root->findCollisionActors(bridgePieces.at(j)->getHitboxForParent(), shared_from_this()));
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
            int num = bridgePieces.size();

            toughness = 0;

            double lowest = (coords.x - posX) / 16;
            int drop = std::min((-abs(lowest - num / 2.0) + (num / 2.0)) * 6, 2.0 * num - toughness);
            
            posY = std::min(originalY + drop, std::max(originalY, coords.y));

            for (int j = 0; j < num; ++j) {
                coords.x = posX + 16 * j;

                if (lowest > 0 && lowest < num) {
                    if (j <= lowest) {
                        coords.y = originalY - 2.0 + pow(j / lowest, 0.7) * drop;
                    } else {
                        coords.y = originalY - 2.0 + pow((num - j) / (num - lowest), 0.7) * drop;
                    }
                } else {
                    coords.y = originalY - 2;
                }

                bridgePieces.at(j)->moveInstantly(coords);
            }
        }

    }

    if (!found) {
        for (int j = 0; j < bridgePieces.size(); ++j) {
            CoordinatePair coords = bridgePieces.at(j)->getPosition();
            coords.y = originalY;
            bridgePieces.at(j)->moveInstantly(coords);
        }
        posY = originalY;
    }
}
