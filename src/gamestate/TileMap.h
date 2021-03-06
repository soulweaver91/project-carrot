#pragma once

#include <memory>
#include <QBitArray>
#include <QVector>
#include <QSettings>
#include <QSet>
#include <SFML/Graphics.hpp>

#include "../graphics/Tileset.h"
#include "../struct/WeaponTypes.h"
#include "../struct/PCEvent.h"
#include "../struct/Hitbox.h"
#include "../struct/AnimState.h"
#include "../struct/Resources.h"
#include "../struct/Layers.h"
#include "../struct/CoordinatePair.h"
#include "../struct/TileCoordinatePair.h"

class ActorAPI;
class GameView;
class AnimatedTile;
class LevelManager;
class EventMap;

class DestructibleDebris {
public:
    DestructibleDebris(std::shared_ptr<sf::Texture> texture, 
        TileCoordinatePair pos, unsigned textureX, unsigned textureY, unsigned short quarter);
    ~DestructibleDebris();
    void tickUpdate();
    void drawUpdate(std::shared_ptr<GameView>& view);
    double getY();

private:
    std::unique_ptr<sf::Sprite> sprite;
    CoordinatePair pos;
    sf::Vector2f speed;
    static const int speedMultiplier[4];
};

class TileMap : public std::enable_shared_from_this<TileMap> {
public:
    TileMap(LevelManager* root, const QString& tilesetFilename, 
        const QString& maskFilename, const QString& sprLayerFilename);
    ~TileMap();
        
    // level related
    void readLevelConfiguration(const QString& filename);
    void readLayerConfiguration(enum LayerType type, const QString& filename, QSettings& config, unsigned layerIdx = 0);
    void readAnimatedTiles(const QString& filename);
    void drawLowerLevels(std::shared_ptr<GameView>& view);
    void drawHigherLevels(std::shared_ptr<GameView>& view);
    unsigned getLevelWidth() const;
    unsigned getLevelHeight() const;
    void setTileEventFlag(const EventMap* events, const TileCoordinatePair& tilePos, PCEvent e = PC_EMPTY);
    SuspendType getPositionSuspendType(const CoordinatePair& pos) const;
    QVector<QVector<std::shared_ptr<LayerTile>>> prepareSavePointLayer();
    void loadSavePointLayer(const QVector<QVector<std::shared_ptr<LayerTile>>>& layer);
    bool checkWeaponDestructible(const CoordinatePair& pos, WeaponType weapon = WEAPON_BLASTER);
    uint checkSpecialDestructible(const Hitbox& hitbox);
    uint checkSpecialSpeedDestructible(const Hitbox& hitbox, const double& speed);
    uint checkCollapseDestructible(const Hitbox& hitbox);
    void saveInitialSpriteLayer();
    void setTrigger(unsigned char triggerID, bool newState);
    bool getTrigger(unsigned char triggerID) const;
    void advanceAnimatedTileTimers();
    void advanceCollapsingTileTimers();
    const std::shared_ptr<sf::Texture> getTilesetTexture() const;
    bool isTileEmpty(const TileCoordinatePair& tilePos) const;
    bool isTileEmpty(const Hitbox& hitbox, bool downwards = false) const;
    void resizeTexturedBackgroundSprite(int width, int height);

private:
    LevelManager* root;
    void drawLayer(TileMapLayer& layer, std::shared_ptr<GameView>& view);
    double translateCoordinate(const double& coordinate, const double& speed, const double& offset, const bool& isY,
        const int& viewHeight, const int& viewWidth) const;
    void updateSprLayerIdx();
    void initializeBackgroundTexture(TileMapLayer& background);
    void drawTexturedBackground(TileMapLayer& layer, const double& x, const double& y, std::shared_ptr<GameView>& view);
    void setTileDestructibleEventFlag(std::shared_ptr<LayerTile>& tile, const TileCoordinatePair& tilePos,
        const TileDestructType& type, const quint16& extraByte);
    bool advanceDestructibleTileAnimation(std::shared_ptr<LayerTile>& tile, const TileCoordinatePair& tilePos, const QString& soundName);
    std::shared_ptr<LayerTile> cloneDefaultLayerTile(const TileCoordinatePair& tilePos);
    std::unique_ptr<Tileset> levelTileset;
    QVector<TileMapLayer> levelLayout;
    unsigned sprLayerIdx;
    QVector<QVector<std::shared_ptr<LayerTile>>> spriteLayerAtLevelStart;
    QVector<std::shared_ptr<AnimatedTile>> animatedTiles;
    bool triggerState[256];
    std::unique_ptr<sf::RenderTexture> texturedBackgroundTexture;
    std::unique_ptr<sf::Sprite> texturedBackgroundSprite;
    unsigned levelWidth;
    unsigned levelHeight;
    sf::Color texturedBackgroundColor;
    std::shared_ptr<ResourceSet> sceneryResources;
    QSet<TileCoordinatePair> activeCollapsingTiles;
};
