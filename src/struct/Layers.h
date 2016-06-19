#pragma once

#include <memory>
#include <QVector>
#include <SFML/Graphics.hpp>

enum LayerType {
    LAYER_SKY_LAYER = 0,
    LAYER_BACKGROUND_LAYER = 1,
    LAYER_SPRITE_LAYER = 2,
    LAYER_FOREGROUND_LAYER = 3
};

enum TileDestructType {
    DESTRUCT_NONE,
    DESTRUCT_WEAPON,
    DESTRUCT_SPEED, // not implemented yet
    DESTRUCT_COLLAPSE, // not implemented yet
    DESTRUCT_SPECIAL, // buttstomp, sidekick, uppercut
    DESTRUCT_TRIGGER
};

enum SuspendType {
    SUSPEND_NONE,
    SUSPEND_VINE,
    SUSPEND_HOOK
};

struct LayerTile {
    uint tileId;
    // Held by the layer tile; conceptually related to the sprite,
    // but the sprite only takes its texture by reference
    std::shared_ptr<sf::Texture> texture;
    std::shared_ptr<sf::Sprite> sprite;
    bool isFlippedX;
    bool isFlippedY;
    bool isAnimated;
    // collision affecting modifiers
    bool isOneWay;
    SuspendType suspendType;
    TileDestructType destructType;
    unsigned long destructAnimation; // animation index for a destructible tile that uses an animation but doesn't animate normally
    uint destructFrameIndex; // denotes the specific frame from the above animation that is currently active
    // Collapsible: delay ("wait" parameter); trigger: trigger id
    unsigned extraByte;
    bool tilesetDefault;
};

struct TileMapLayer {
    enum LayerType type;
    unsigned idx;
    QVector<QVector<std::shared_ptr<LayerTile>>> tileLayout;
    double speedX;
    double speedY;
    double autoSpeedX;
    double autoSpeedY;
    bool repeatX;
    bool repeatY;
    double offsetX;
    double offsetY;
    // JJ2's "limit visible area" flag
    bool useInherentOffset;

    // textured background, only useful for sky layer
    bool isTextured;
    bool useStarsTextured;
    sf::Color texturedBackgroundColor;

    bool operator< (TileMapLayer layer) {
        if (type != layer.type) {
            return (type < layer.type);
        }
        return (idx < layer.idx);
    };
};
