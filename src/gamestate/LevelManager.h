#pragma once

#include <memory>
#include <functional>
#include <QVector>
#include <QString>
#include <QObject>
#include "../ModeManager.h"
#include "../struct/CoordinatePair.h"
#include "../struct/Hitbox.h"
#include "../struct/NextLevelData.h"
#include "../struct/Resources.h"
#include "../struct/DebugConfig.h"
#include "TimerUser.h"

class CarrotQt5;
class CommonActor;
class ActorAPI;
class Player;
class SolidObject;
class TileMap;
class EventMap;
class DestructibleDebris;
class GameView;
class BitmapString;
class BitmapFont;
class SoundSystem;
struct LayerTile;

struct SavedState {
    CoordinatePair playerPosition;
    int playerLives;
    QVector<QVector<std::shared_ptr<LayerTile>>> spriteLayerState;
};

class LevelManager : public QObject, public ModeManager, public TimerUser, public std::enable_shared_from_this<LevelManager> {

    Q_OBJECT

public:
    LevelManager(CarrotQt5* root, const QString& level, const QString& episode);
    ~LevelManager();

    void tick(const ControlEventList& events) override;

    bool addActor(std::shared_ptr<CommonActor> actor);
    bool addPlayer(std::shared_ptr<Player> actor, short playerID = -1);
    void removeActor(std::shared_ptr<CommonActor> actor);
    QVector<std::weak_ptr<CommonActor>> findCollisionActors(const Hitbox& hitbox, std::shared_ptr<CommonActor> me = nullptr);
    QVector<std::weak_ptr<CommonActor>> findCollisionActors(std::shared_ptr<CommonActor> me = nullptr);
    void setSavePoint();
    void loadSavePoint();
    void clearActors();
    void createDebris(unsigned tileId, int x, int y);
    bool isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me, std::weak_ptr<SolidObject>& collisionActor);
    bool isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me);
    QVector<std::weak_ptr<Player>> getCollidingPlayer(const Hitbox& hitbox);
    std::weak_ptr<Player> getPlayer(unsigned number);
    std::weak_ptr<TileMap> getGameTiles();
    std::weak_ptr<EventMap> getGameEvents();
    uint getDefaultLightingLevel();
    double getGravity();
    void resizeEvent(int w, int h);
    void initLevelChange(ExitType e = NEXT_NORMAL);
    std::shared_ptr<ActorAPI> getActorAPI();
    void processCarryOver(const LevelCarryOver carryOver);

#ifdef CARROT_DEBUG
public slots:
    void debugSetGravity();
    void debugSetLighting();
    void debugSetPosition();
    void debugSugarRush();
#endif

private:
    CarrotQt5* root;
    std::shared_ptr<ActorAPI> api;
    void setLevelName(const QString& name);
    void cleanUpLevel();
    void processControlEvents(const ControlEventList& events);
    std::function<void(QString)> drawLoadingScreen(const QString& levelName);

    QVector<std::shared_ptr<CommonActor>> actors;
    QVector<std::shared_ptr<DestructibleDebris>> debris;
    std::shared_ptr<Player> players[32];
    QVector<std::shared_ptr<GameView>> views;
    std::shared_ptr<TileMap> gameTiles;
    std::shared_ptr<EventMap> gameEvents;
    QString levelName;
    QString episodeName;
    QString nextLevel;
    SavedState lastSavePoint;
    bool exiting;
    uint defaultLightingLevel;
    double gravity;
};