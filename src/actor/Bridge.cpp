#include "Bridge.h"

#include <cmath>
#include "Player.h"
#include "../gamestate/ActorAPI.h"
#include "../gamestate/EventMap.h"

DynamicBridgePiece::DynamicBridgePiece(std::shared_ptr<ActorAPI> api, double x, double y, DynamicBridgeType type, uint idx)
    : SolidObject(api, x, y, false) {
    canBeFrozen = false;
    loadResources("Object/BridgePiece");

    QString typeName = "ROPE";
    switch (type) {
        case BRIDGE_GEM:       typeName = "GEM";       break;
        case BRIDGE_LAB:       typeName = "LAB";       break;
        case BRIDGE_LOG:       typeName = "LOG";       break;
        case BRIDGE_ROPE:      typeName = "ROPE";      break;
        case BRIDGE_STONE:     typeName = "STONE";     break;
        case BRIDGE_STONE_RED: typeName = "STONE_RED"; break;
        case BRIDGE_VINE:      typeName = "VINE";      break;
    }
    AnimationUser::setAnimation("OBJECT_BRIDGE_PIECE_" + typeName);

    uint variations = currentAnimation->getAnimation()->frameCount;
    if (variations > 0) {
        currentAnimation->resetFrame(idx % variations);
    }
    
    isGravityAffected = false;
    isOneWay = true;
}

DynamicBridgePiece::~DynamicBridgePiece() {

}


bool DynamicBridgePiece::deactivate(int, int, int) {
    // Removal of bridge pieces is handled by the bridge event.
    return false;
}

Hitbox DynamicBridgePiece::getHitboxForParent() {
    return Hitbox(currentHitbox).extend(0, 4);
}

void DynamicBridgePiece::tryStandardMovement() {
    // The bridge piece doesn't move by itself.
}

DynamicBridge::DynamicBridge(std::shared_ptr<ActorAPI> api, double x, double y, unsigned int width,
    DynamicBridgeType type, unsigned int toughness)
    : CommonActor(api, x - 16.0, y - 16.0), originalY(y - 8.0), bridgeType(type), bridgeWidth(width),
    heightFactor((16.0 - toughness) * bridgeWidth) {
    loadResources("Object/Bridge");

    if ((uint)bridgeType > (uint)BRIDGE_LAB) {
        bridgeType = BRIDGE_ROPE;
    }

    auto& widthList = BRIDGE_PIECE_WIDTHS.value(bridgeType, { 16 });
    uint widthCovered = widthList[0] / 2;
    for (uint i = 0; (widthCovered <= width * 16) || (i * 16 < width); ++i) {
        auto piece_n = std::make_shared<DynamicBridgePiece>(api, x + widthCovered - 16, y - 20, type, i);
        api->addActor(piece_n);
        bridgePieces << piece_n;
        widthCovered += (widthList[i % widthList.size()] + widthList[(i + 1) % widthList.size()]) / 2;
    }

    isGravityAffected = false;
    updateHitbox();
}

DynamicBridge::~DynamicBridge() {

}

bool DynamicBridge::deactivate(int x, int y, int dist) {
    auto events = api->getGameEvents().lock();

    if ((std::abs(x - originTileX) > dist) || (std::abs(y - originTileY) > dist)) {
        if (events != nullptr) {
            events->deactivate(originTileX, originTileY);
        }

        for (int i = 0; i < bridgePieces.size(); ++i) {
            api->removeActor(bridgePieces.at(i));
        }
        api->removeActor(shared_from_this());
        return true;
    }
    return false;
}

void DynamicBridge::updateHitbox() {
    currentHitbox = Hitbox(posX, posY - 10.0, posX + bridgeWidth * 16.0, posY + 16.0);
}

void DynamicBridge::tickEvent() {
    auto collision = api->findCollisionActors(currentHitbox, shared_from_this());
    for (int j = 0; j < bridgePieces.size(); ++j) {
        collision.append(api->findCollisionActors(bridgePieces.at(j)->getHitboxForParent(), shared_from_this()));
    }

    bool found = false;
    for (int i = 0; i < collision.size(); ++i) {
        auto collisionPtr = collision.at(i).lock();
        if (collisionPtr == nullptr) {
            continue;
        }

        // TODO: This code only works with one player
        // PC is single player for now so that can be dealt with later
        auto p = std::dynamic_pointer_cast<Player>(collisionPtr);
        if (p != nullptr) {
            found = true;
            CoordinatePair coords = p->getPosition();
            int num = bridgePieces.size();

            // This marks which bridge piece is under the player and should be positioned
            // lower than any other piece of the bridge.
            double lowest = (coords.x - posX) / (bridgeWidth * 16.0) * num;

            // This marks the maximum drop in height. 
            // At the middle of the bridge, this is purely the height factor,
            // which is simply (16 - bridge toughness) multiplied by the length of the bridge.
            // At other points, the height is scaled by an (arbitrarily chosen) power that
            // gives a nice curve.
            // Additionally, the drop is reduced based on the player position so that the
            // bridge seems to bend somewhat realistically instead of snapping from one position
            // to another.
            double drop = std::max(0.0, std::min(coords.y - posY + 32.0, (1.0 - std::pow(std::abs(2.0 * lowest / num - 1.0), 0.7)) * heightFactor));
            
            posY = std::min(originalY + drop, std::max(originalY, coords.y));

            // Update the position of each bridge piece.
            for (int j = 0; j < num; ++j) {
                auto piece = bridgePieces.at(j);
                coords.x = piece->getPosition().x;

                if (lowest > 0 && lowest < num) {
                    if (j <= lowest) {
                        coords.y = originalY + std::pow(j / lowest, 0.7) * drop;
                    } else {
                        coords.y = originalY + std::pow((num - 1 - j) / (num - 1 - lowest), 0.7) * drop;
                    }
                } else {
                    coords.y = originalY;
                }

                piece->moveInstantly(coords, true, true);
            }
        }

    }

    if (!found) {
        // The player was not touching the bridge, so reset all pieces to the default height.
        for (int j = 0; j < bridgePieces.size(); ++j) {
            CoordinatePair coords = bridgePieces.at(j)->getPosition();
            coords.y = originalY;
            bridgePieces.at(j)->moveInstantly(coords, true, true);
        }
        posY = originalY;
    }
}

// TODO: Maybe figure out a way to externalise this outside of the code later.
// Separate frame sizes kinda exist in Anims.j2a but they are not stored after
// the conversion and for 99% of the assets they would be useless cruft.
const QMap<DynamicBridgeType, QVector<uint>> DynamicBridge::BRIDGE_PIECE_WIDTHS = {
    { BRIDGE_GEM, { 14 }},
    { BRIDGE_LAB, { 14 }},
    { BRIDGE_LOG, { 13, 13, 13 }},
    { BRIDGE_STONE, { 15, 9, 10, 9, 15, 9, 15 }},
    { BRIDGE_STONE_RED, { 10, 11, 11, 12 }},
    { BRIDGE_ROPE, { 13, 13, 10, 13, 13, 12, 11 }},
    { BRIDGE_VINE, { 7, 7, 7, 7, 10, 7, 7, 7, 7 }}
};