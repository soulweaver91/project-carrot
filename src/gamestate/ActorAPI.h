#pragma once

#include <memory>

#include "LevelManager.h"
#include "../CarrotQt5.h"
#include "../struct/DebugConfig.h"
#include "../struct/PCEvent.h"
#include "../struct/TileCoordinatePair.h"

class ActorAPI : public std::enable_shared_from_this<ActorAPI> {
public:
    ActorAPI(CarrotQt5* mainClass, LevelManager* levelManager);
    ~ActorAPI();

    bool addActor(std::shared_ptr<CommonActor> actor);
    void removeActor(std::shared_ptr<CommonActor> actor);
    std::shared_ptr<CommonActor> createActor(PCEvent type, const CoordinatePair& pos, const quint16 params[8], bool returnOnly = false);

    QVector<std::weak_ptr<CommonActor>> findCollisionActors(const Hitbox& hitbox, std::shared_ptr<CommonActor> me = nullptr);
    QVector<std::weak_ptr<CommonActor>> findCollisionActors(std::shared_ptr<CommonActor> me = nullptr);
    void setSavePoint();
    void loadSavePoint();
    bool isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me, std::weak_ptr<SolidObject>& collisionActor);
    bool isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me);
    bool isTileCollisionFree(const TileCoordinatePair& tile);
    QVector<std::weak_ptr<Player>> getCollidingPlayer(const Hitbox& hitbox);
    std::weak_ptr<Player> getPlayer(unsigned number);
    uint getDefaultLightingLevel();
    double getGravity();
    void initLevelChange(ExitType e = NEXT_NORMAL);
    unsigned long getFrame();
    std::shared_ptr<BitmapString> makeString(const QString& initString = "", BitmapFontSize size = NORMAL, FontAlign initAlign = FONT_ALIGN_LEFT);
    uint getStringWidth(const QString& text, BitmapFontSize size = NORMAL);
    QString getLevelText(int idx);
    void handleGameOver();
    quint32 getLevelTileWidth();
    quint32 getLevelTileHeight();
    void setTrigger(unsigned char triggerID, bool newState);

    template<typename... P>
    void playSound(HSAMPLE sample, P... params);

    void pauseMusic();
    void resumeMusic();

#ifdef CARROT_DEBUG
    DebugConfig* getDebugConfig();
#endif

    // TODO: phase out with better API functions
    std::weak_ptr<TileMap> getGameTiles();
    std::weak_ptr<EventMap> getGameEvents();
    std::shared_ptr<ResourceSet> loadActorTypeResources(const QString& classId);

private:
    CarrotQt5* const mainClass;
    LevelManager* const levelManager;
};
