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
#include <QVector>
#include <bass.h>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include "ui_carrotqt5.h"
#include "graphics/BitmapFont.h"
#include "graphics/CarrotCanvas.h"
#include "graphics/ShaderSource.h"
#include "gamestate/ControlManager.h"
#include "gamestate/ResourceManager.h"
#include "struct/Controls.h"
#include "struct/CoordinatePair.h"
#include "struct/Hitbox.h"
#include "struct/Resources.h"

class CommonActor;
class Player;
class SolidObject;
class TileMap;
class EventMap;
class DestructibleDebris;
class MenuScreen;
class CarrotQt5; // forward declaration required for the typedef to work
struct LayerTile;

enum ExitType {
    NEXT_NONE,
    NEXT_NORMAL,
    NEXT_BONUS,
    NEXT_SPECIAL
};

struct SavedState {
    CoordinatePair playerPosition;
    int playerLives;
    QVector<QVector<std::shared_ptr<LayerTile>>> spriteLayerState;
};

typedef void (CarrotQt5::*InvokableRootFunction)(QVariant);

class CarrotQt5 : public QMainWindow, public std::enable_shared_from_this<CarrotQt5> {

    Q_OBJECT

public:
    CarrotQt5(QWidget *parent = 0);
    ~CarrotQt5();
    void parseCommandLine();
    void startMainMenu();
    unsigned getLevelWidth();
    unsigned getLevelHeight();
    unsigned getViewWidth();
    unsigned getViewHeight();
    CoordinatePair getViewCenter();
    void centerView(CoordinatePair pos);
    void centerView(double x, double y);
    bool addActor(std::shared_ptr<CommonActor> actor);
    bool addPlayer(std::shared_ptr<Player> actor, short playerID = -1);
    void removeActor(std::shared_ptr<CommonActor> actor);
    bool loadLevel(const QString& name);
    QVector<std::weak_ptr<CommonActor>> findCollisionActors(CoordinatePair pos, std::shared_ptr<CommonActor> me = nullptr);
    QVector<std::weak_ptr<CommonActor>> findCollisionActors(Hitbox hitbox, std::shared_ptr<CommonActor> me = nullptr);
    static Hitbox calcHitbox(const Hitbox& hitbox, double diffX, double diffY);
    static Hitbox calcHitbox(int x, int y, int w, int h);
    void setSavePoint();
    void loadSavePoint();
    void clearActors();
    unsigned long getFrame();
    void createDebris(unsigned tileId, int x, int y);
    void setLighting(int target, bool immediate);
    void initLevelChange(ExitType e = NEXT_NORMAL);
    bool isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me, std::weak_ptr<SolidObject>& collisionActor);
    bool isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me);
    std::weak_ptr<CarrotCanvas> getCanvas();
    std::shared_ptr<BitmapFont> getFont();
    std::weak_ptr<SoundSystem> getSoundSystem();
    std::weak_ptr<Player> getPlayer(unsigned number);
    std::weak_ptr<TileMap> getGameTiles();
    std::weak_ptr<EventMap> getGameEvents();
    std::shared_ptr<ResourceSet> loadActorTypeResources(const QString& actorType);
    std::shared_ptr<ShaderSource> getShaderSource();
    int getLightingLevel();
    double gravity;
    bool dbgShowMasked;
    int tempModifier[32]; // temporary variables for testing new features
    QString tempModifierName[32];
    unsigned char currentTempModifier;

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
    void spawnEventMap(const QString& filename, unsigned layoutVersion = 1);
    void setLevelName(const QString& name);
    void cleanUpLevel();
    void processControlEvents();
    Ui::CarrotQt5Class ui;
    QTimer myTimer;
    QVector<std::shared_ptr<CommonActor>> actors;
    QVector<std::shared_ptr<DestructibleDebris>> debris;
    std::shared_ptr<Player> players[32];
    bool paused;
    std::unique_ptr<sf::View> uiView;
    std::unique_ptr<sf::Texture> pausedScreenshot;
    std::unique_ptr<sf::RenderTexture> lightTexture;
    std::unique_ptr<sf::Sprite> pausedScreenshotSprite;
    std::unique_ptr<BitmapString> pausedText;
    std::shared_ptr<CarrotCanvas> windowCanvas;
    std::shared_ptr<BitmapFont> mainFont;
    std::shared_ptr<TileMap> gameTiles;
    std::shared_ptr<EventMap> gameEvents;
    std::shared_ptr<ShaderSource> shaderSource;
    std::unique_ptr<sf::View> gameView;
    std::unique_ptr<ResourceManager> resourceManager;
    std::shared_ptr<ControlManager> controlManager;
    QString levelName;
    QString nextLevel;
    unsigned long frame;
    SavedState lastSavePoint;
    int lightingLevel;
    int targetLightingLevel;
    ExitType lastExit;
    std::shared_ptr<MenuScreen> menuObject;
    bool isMenu;
    QTime lastTimestamp;
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
