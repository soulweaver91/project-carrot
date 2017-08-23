#pragma once

#include <memory>
#include <QVector>
#include <QMap>
#include "SolidObject.h"
#include "../../struct/Hitbox.h"

class ActorAPI;

enum DynamicBridgeType {
    BRIDGE_ROPE       = 0,
    BRIDGE_STONE      = 1,
    BRIDGE_VINE       = 2,
    BRIDGE_STONE_RED  = 3,
    BRIDGE_LOG        = 4,
    BRIDGE_GEM        = 5,
    BRIDGE_LAB        = 6
    // type 7 in JJ2 equals to 0
};

class DynamicBridgePiece : public SolidObject {
public:
    DynamicBridgePiece(const ActorInstantiationDetails& initData, DynamicBridgeType type = BRIDGE_ROPE, uint idx = 0);
    ~DynamicBridgePiece();
    bool deactivate(const TileCoordinatePair& tilePos, int dist) override;
    Hitbox getHitboxForParent();

private:
    void tryStandardMovement() override;
};

class DynamicBridge : public CommonActor {
public:
    DynamicBridge(const ActorInstantiationDetails& initData, unsigned int width = 1,
        DynamicBridgeType type = BRIDGE_ROPE, unsigned int toughness = 0);
    ~DynamicBridge();
    bool deactivate(const TileCoordinatePair& tilePos, int dist) override;
    void updateHitbox() override;
    void tickEvent() override;

private:
    double originalY;
    DynamicBridgeType bridgeType;
    uint bridgeWidth;
    uint heightFactor;
    QVector<std::shared_ptr<DynamicBridgePiece>> bridgePieces;

    static const QMap<DynamicBridgeType, QVector<uint>> BRIDGE_PIECE_WIDTHS;
};
