#pragma once

#include <memory>
#include <QBitArray>
#include <QVector>
#include <QSettings>
#include <SFML/Graphics.hpp>

#include "../struct/WeaponTypes.h"
#include "../struct/PCEvent.h"
#include "../struct/Hitbox.h"
#include "../struct/AnimState.h"
#include "../struct/Resources.h"

class CarrotQt5;
class AnimatedTile;

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

struct LayerTile {
    unsigned long tileId;
    // Held by the layer tile; conceptually related to the sprite,
    // but the sprite only takes its texture by reference
    std::shared_ptr<sf::Texture> texture;
    std::shared_ptr<sf::Sprite> sprite;
    bool isFlippedX;
    bool isFlippedY;
    bool isAnimated;
    // collision affecting modifiers
    bool isOneWay;
    bool isVine;
    TileDestructType destructType;
    unsigned long destructAnimation; // animation index for a destructible tile that uses an animation but doesn't animate normally
    int destructFrameIndex; // denotes the specific frame from the above animation that is currently active
    // Collapsible: delay ("wait" parameter); trigger: trigger id
    unsigned extraByte;
    bool tilesetDefault;
};

struct Tileset {
    QString name;
    unsigned long tileCount;
    unsigned tilesPerRow; // number of tiles next to each other
    std::shared_ptr<sf::Texture> tiles;
    QVector<QBitArray> masks;
    QVector<bool> isMaskEmpty; // to speed up collision checking so that not every tile needs to be pixel perfect checked
    QVector<bool> isMaskFilled;  // same with this one
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

    bool TileMapLayer::operator< (TileMapLayer layer) {
        if (type != layer.type) {
            return (type < layer.type);
        }
        return (idx < layer.idx);
    };
};

class DestructibleDebris {
public:
    DestructibleDebris(std::shared_ptr<sf::Texture> texture, std::weak_ptr<sf::RenderTarget> window, 
        int x, int y, unsigned textureX, unsigned textureY, unsigned short quarter);
    ~DestructibleDebris();
    void tickUpdate();
    double getY();

private:
    std::unique_ptr<sf::Sprite> sprite;
    std::weak_ptr<sf::RenderTarget> window;
    double posX;
    double posY;
    double speedX;
    double speedY;
    static const int speedMultiplier[4];
};

class TileMap : public std::enable_shared_from_this<TileMap> {
public:
    TileMap(std::shared_ptr<CarrotQt5> gameRoot, const QString& tilesetFilename, const QString& maskFilename, const QString& sprLayerFilename);
    ~TileMap();
        
    // level related
    void readLevelConfiguration(const QString& filename);
    void readLayerConfiguration(enum LayerType type, const QString& filename, unsigned layerIdx = 0, QSettings& config = QSettings());
    void readAnimatedTiles(const QString& filename);
    void drawLowerLevels();
    void drawHigherLevels();
    unsigned getLevelWidth();
    unsigned getLevelHeight();
    void setTileEventFlag(int x, int y, PCEvent e = PC_EMPTY);
    bool isPosVine(double x, double y);
    QVector<QVector<std::shared_ptr<LayerTile>>> prepareSavePointLayer();
    void loadSavePointLayer(const QVector<QVector<std::shared_ptr<LayerTile>>>& layer);
    bool checkWeaponDestructible(double x, double y, WeaponType weapon = WEAPON_BLASTER);
    bool checkSpecialDestructible(double x, double y);
    void saveInitialSpriteLayer();
    void setTrigger(unsigned char triggerID, bool newState);
    bool getTrigger(unsigned char triggerID);
    void advanceAnimatedTileTimers();

    // assigned tileset related
    const std::shared_ptr<sf::Texture> getTilesetTexture();
    void readTileset(const QString& tilesFilename, const QString& maskFilename);
    bool isTileEmpty(unsigned x, unsigned y);
    bool isTileEmpty(const Hitbox& hitbox, bool downwards = false);

private:
    std::shared_ptr<CarrotQt5> root;
    void drawLayer(TileMapLayer& layer, std::shared_ptr<sf::RenderWindow> target);
    double translateCoordinate(const double& coordinate, const double& speed, const double& offset, const bool& isY) const;
    void updateSprLayerIdx();
    void initializeBackgroundTexture(TileMapLayer& background);
    void drawTexturedBackground(TileMapLayer& layer, const double& x, const double& y, std::shared_ptr<sf::RenderWindow> target);
    Tileset levelTileset;
    QVector<TileMapLayer> levelLayout;
    unsigned sprLayerIdx;
    QVector<QVector<std::shared_ptr<LayerTile>>> spriteLayerAtLevelStart;
    QVector<std::shared_ptr<AnimatedTile>> animatedTiles;
    bool triggerState[256];
    std::unique_ptr<sf::RenderTexture> texturedBackgroundTexture;
    std::unique_ptr<sf::VertexArray> texturedBackgroundFadeArray;
    unsigned levelWidth;
    unsigned levelHeight;
    std::shared_ptr<ResourceSet> sceneryResources;
    QVector<std::shared_ptr<LayerTile>> defaultLayerTiles;
    std::shared_ptr<LayerTile> cloneDefaultLayerTile(int x, int y);
};
