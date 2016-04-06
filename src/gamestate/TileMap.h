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
    unsigned long tile_id;
    // Held by the layer tile; conceptually related to the sprite,
    // but the sprite only takes its texture by reference
    std::shared_ptr<sf::Texture> texture;
    std::shared_ptr<sf::Sprite> sprite;
    bool flipped_x;
    bool flipped_y;
    bool animated;
    // collision affecting modifiers
    bool oneway;
    bool vine;
    TileDestructType dtype;
    unsigned long d_animation; // animation index for a destructible tile that uses an animation but doesn't animate normally
    int scenery_frame_idx; // denotes the specific frame from the above animation that is currently active
    // Collapsible: delay ("wait" parameter); trigger: trigger id
    unsigned extra_byte;
    bool tilesetDefault;
};

struct Tileset {
    QString unique_id;
    QString name;
    unsigned long tile_amount;
    unsigned tiles_col; // number of tiles next to each other
    std::shared_ptr<sf::Texture> tiles;
    QVector< QBitArray > masks;
    QVector< bool > mask_empty; // to speed up collision checking so that not every tile needs to be pixel perfect checked
    QVector< bool > mask_full;  // same with this one
};

struct TileMapLayer {
    enum LayerType type;
    unsigned idx;
    QVector<QVector<std::shared_ptr<LayerTile>>> tile_layout;
    double xspeed;
    double yspeed;
    double auto_xspeed;
    double auto_yspeed;
    bool xrepeat;
    bool yrepeat;
    double xoffset;
    double yoffset;
    // JJ2's "limit visible area" flag
    bool inherent_offset;

    // textured background, only useful for sky layer
    bool textured;
    bool textured_stars;
    sf::Color textured_color;

    bool TileMapLayer::operator< (TileMapLayer layer) {
        if (type != layer.type) {
            return (type < layer.type);
        }
        return (idx < layer.idx);
    };
};

class DestructibleDebris {
    public:
        DestructibleDebris(std::shared_ptr<sf::Texture> tex, std::weak_ptr<sf::RenderTarget> win, 
            int x, int y, unsigned tx, unsigned ty, unsigned short quarter);
        ~DestructibleDebris();
        void TickUpdate();
        double GetY();
    private:
        std::unique_ptr<sf::Sprite> spr;
        std::weak_ptr<sf::RenderTarget> wint;
        double pos_x;
        double pos_y;
        double h_speed;
        double v_speed;
};

class TileMap : public std::enable_shared_from_this<TileMap> {
    public:
        TileMap(std::shared_ptr<CarrotQt5> game_root, const QString& tileset_file, const QString& mask_file, const QString& spr_layer_file);
        ~TileMap();
        
        // level related
        void readLevelConfiguration(const QString& filename);
        void readLayerConfiguration(enum LayerType type, const QString& filename, unsigned layer_idx = 0, QSettings& config = QSettings());
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
        void setTrigger(unsigned char trigger_id,bool new_state);
        bool getTrigger(unsigned char trigger_id);
        void advanceAnimatedTileTimers();

        // assigned tileset related
        const std::shared_ptr<sf::Texture> getTilesetTexture();
        void readTileset(const QString& file_tiles, const QString& file_mask);
        bool isTileEmpty(unsigned x, unsigned y);
        bool isTileEmpty(const Hitbox& hbox, bool downwards = false);
    private:
        std::shared_ptr<CarrotQt5> root;
        void drawLayer(TileMapLayer& layer, std::shared_ptr<sf::RenderWindow> target);
        double translateCoordinate(const double& coordinate, const double& speed, const double& offset, const bool& is_y) const;
        void updateSprLayerIdx();
        void initializeBackgroundTexture(TileMapLayer& background);
        void drawTexturedBackground(TileMapLayer& layer, const double& x, const double& y, std::shared_ptr<sf::RenderWindow> target);
        Tileset level_tileset;
        QVector< TileMapLayer > level_layout;
        unsigned spr_layer;
        QVector<QVector<std::shared_ptr<LayerTile>>> initial_spr_layer_copy;
        QVector<std::shared_ptr<AnimatedTile>> animated_tiles;
        bool trigger_state[256];
        std::unique_ptr<sf::RenderTexture> tex_back;
        std::unique_ptr<sf::VertexArray> tex_fade;
        unsigned level_width;
        unsigned level_height;
        std::shared_ptr<ResourceSet> sceneryResources;
        QVector<std::shared_ptr<LayerTile>> defaultLayerTiles;
        std::shared_ptr<LayerTile> cloneDefaultLayerTile(int x, int y);
};
