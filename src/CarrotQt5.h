/*****************************************************************************
 ** PROJECT CARROT - MAIN CLASS                                             **
 *****************************************************************************/

#pragma once
#include "Version.h"

#include <memory>
#include <QMainWindow>
#include <QCloseEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QTimer>
#include <QTime>
#include <QString>
#include <QVariant>
#include <QWidget>
#include <QObject>
#include <QMap>
#include <bass.h>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "ui_carrotqt5.h"
#include "graphics/BitmapFont.h"
#include "graphics/CarrotCanvas.h"
#include "sound/SoundFX.h"

class CommonActor;
class Player;
class SolidObject;
class TileMap;
class EventMap;
class DestructibleDebris;
class MenuScreen;
class CarrotQt5; // forward declaration required for the typedef to work
struct LayerTile;

// These need to match the other event tables (at the moment only at Collectible.h) where applicable
// because created objects are directly cast from these values in the event spawner
enum PCEvent {
    PC_EMPTY                = 0x0000,
    PC_FAST_FIRE            = 0x0001,
    PC_AMMO_BOUNCER         = 0x0002,
    PC_AMMO_FREEZER         = 0x0003, // not implemented
    PC_AMMO_SEEKER          = 0x0004,
    PC_AMMO_RF              = 0x0005, // not implemented
    PC_AMMO_TOASTER         = 0x0006,
    PC_AMMO_TNT             = 0x0007, // not implemented
    PC_AMMO_PEPPER          = 0x0008, // not implemented
    PC_AMMO_ELECTRO         = 0x0009, // not implemented
    PC_POWERUP_BLASTER      = 0x0011, // not implemented
    PC_POWERUP_BOUNCER      = 0x0012, // not implemented
    PC_POWERUP_FREEZER      = 0x0013, // not implemented
    PC_POWERUP_SEEKER       = 0x0014, // not implemented
    PC_POWERUP_RF           = 0x0015, // not implemented
    PC_POWERUP_TOASTER      = 0x0016, // not implemented
    PC_POWERUP_TNT          = 0x0017, // not implemented
    PC_POWERUP_PEPPER       = 0x0018, // not implemented
    PC_POWERUP_ELECTRO      = 0x0019, // not implemented
    PC_GEM_RED              = 0x0040,
    PC_GEM_GREEN            = 0x0041,
    PC_GEM_BLUE             = 0x0042,
    PC_SAVE_POINT           = 0x005A,
    PC_JAZZ_LEVEL_START     = 0x0100,
    PC_SPAZ_LEVEL_START     = 0x0101, // not implemented
    PC_LORI_LEVEL_START     = 0x0102, // not implemented
    PC_AREA_EOL             = 0x0108,
    PC_WARP_ORIGIN          = 0x010E,
    PC_WARP_TARGET          = 0x010F,
    PC_MODIFIER_VINE        = 0x0110,
    PC_MODIFIER_ONE_WAY     = 0x0111,
    PC_MODIFIER_HOOK        = 0x0112, // not implemented
    PC_MODIFIER_H_POLE      = 0x0113,
    PC_MODIFIER_V_POLE      = 0x0114,
    PC_SCENERY_DESTRUCT     = 0x0115,
    PC_SCENERY_BUTTSTOMP    = 0x0117,
    PC_MODIFIER_HURT        = 0x0118,
    PC_LIGHT_SET            = 0x0120,
    PC_LIGHT_RESET          = 0x0125,
    PC_ENEMY_TURTLE_NORMAL  = 0x0180,
    PC_ENEMY_LIZARD         = 0x0184,
    PC_PUSHABLE_ROCK        = 0x005C,
    PC_TRIGGER_CRATE        = 0x005B,
    PC_TRIGGER_AREA         = 0x0119,
    PC_MODIFIER_RICOCHET    = 0x011A,
    PC_BRIDGE               = 0xF000, // temp. assignment
    PC_AREA_STOP_ENEMY      = 0xF001, // temp. assignment
    PC_COIN_SILVER          = 0xF002, // temp. assignment
    PC_COIN_GOLD            = 0xF003 // temp. assignment
};

enum ExitType {
    NEXT_NONE,
    NEXT_NORMAL,
    NEXT_BONUS,
    NEXT_SPECIAL
};

struct CoordinatePair {
    double x;
    double y;
};

struct Hitbox {
    double left;
    double top;
    double right;
    double bottom;
};

struct SavedState {
    CoordinatePair player_pos;
    int player_lives;
    QList< QList< LayerTile > > spr_layer_copy;
};

typedef void (CarrotQt5::*InvokableRootFunction)(QVariant);

class CarrotQt5 : public QMainWindow, public std::enable_shared_from_this<CarrotQt5>
{
    Q_OBJECT

public:
    CarrotQt5(QWidget *parent = 0);
    ~CarrotQt5();
    void parseCommandLine();
    void startMainMenu();
    bool setMusic(const QString& filename);
    sf::View* game_view;
    unsigned getLevelWidth();
    unsigned getLevelHeight();
    unsigned getViewWidth();
    unsigned getViewHeight();
    bool addActor(CommonActor* actor);
    bool addPlayer(Player* actor, short player_id = -1);
    void removeActor(CommonActor* actor);
    bool destroyAllActors();
    bool loadLevel(const QString& name);
    QList< CommonActor* > findCollisionActors(CoordinatePair pos, CommonActor* me = nullptr);
    QList< CommonActor* > findCollisionActors(Hitbox hbox, CommonActor* me = nullptr);
    static Hitbox calcHitbox(const Hitbox& hbox, double hor, double ver);
    static Hitbox calcHitbox(int x, int y, int w, int h);
    void setSavePoint();
    void loadSavePoint();
    void clearActors();
    unsigned long getFrame();
    void createDebris(unsigned tile_id, int x, int y);
    void setLighting(int target, bool immediate);
    void initLevelChange(ExitType e = NEXT_NORMAL);
    bool isPositionEmpty(const Hitbox& hbox, bool downwards, CommonActor* me, SolidObject*& collided);
    bool isPositionEmpty(const Hitbox& hbox, bool downwards, CommonActor* me);
    Player* getPlayer(unsigned no);
    int getLightingLevel();
    sf::Texture* getCachedTexture(const QString& filename);
    TileMap* game_tiles;
    EventMap* game_events;
    BitmapFont* mainFont;
    CarrotCanvas* window;
    SFXSystem* sfxsys;
    double gravity;
    bool dbgShowMasked;
    int mod_temp[32]; // temporary variables for testing new features
    QString mod_name[32];
    unsigned char mod_current;

    // remote function evoke
    void invokeFunction(InvokableRootFunction func, QVariant param);
    void CarrotQt5::startGame(QVariant filename);
    void CarrotQt5::quitFromMainMenu(QVariant param);
    
protected:
    bool eventFilter(QObject *watched, QEvent *e);

protected slots:
    void closeEvent(QCloseEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

private:
    void spawnEventMap(const QString& filename, unsigned layout_version = 1);
    void setLevelName(const QString& name);
    void cleanUpLevel();
    void clearTextureCache();
    Ui::CarrotQt5Class ui;
    QTimer myTimer;
    HMUSIC currentMusic;
    QList< CommonActor* > actors;
    QList< DestructibleDebris* > debris;
    Player* players[32];
    bool paused;
    std::unique_ptr<sf::View> uiView;
    std::unique_ptr<sf::Texture> windowTexture;
    std::unique_ptr<sf::RenderTexture> lightTexture;
    std::unique_ptr<sf::Sprite> pauseCap;
    std::unique_ptr<BitmapString> pausedText;
    QString levelName;
    QString nextLevel;
    unsigned long frame;
    SavedState lastSavePoint;
    int lightingLevel;
    int targetLightingLevel;
    ExitType last_exit;
    QMap<QString, std::shared_ptr<sf::Texture>> textureCache;
    std::shared_ptr<MenuScreen> menuObject;
    bool isMenu;
    QTime last_timestamp;
    float fps;

private slots:
    void gameTick();
    void mainMenuTick();
    void openAboutCarrot();
    void openHomePage();
    void debugLoadMusic();
    void debugShowMasks(bool show);
    void debugSetGravity();
    void debugSetLighting();
    void setLightingStep();
    void delayedLevelChange();
};
