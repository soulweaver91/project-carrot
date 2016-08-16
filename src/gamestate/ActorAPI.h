#pragma once

#include <memory>

#include "LevelManager.h"
#include "../CarrotQt5.h"
#include "../struct/DebugConfig.h"

class ActorAPI : public std::enable_shared_from_this<ActorAPI> {
public:
    ActorAPI(CarrotQt5* mainClass, LevelManager* levelManager);
    ~ActorAPI();

    bool addActor(std::shared_ptr<CommonActor> actor);
    void removeActor(std::shared_ptr<CommonActor> actor);
    QVector<std::weak_ptr<CommonActor>> findCollisionActors(const Hitbox& hitbox, std::shared_ptr<CommonActor> me = nullptr);
    QVector<std::weak_ptr<CommonActor>> findCollisionActors(std::shared_ptr<CommonActor> me = nullptr);
    void setSavePoint();
    void loadSavePoint();
    bool isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me, std::weak_ptr<SolidObject>& collisionActor);
    bool isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me);
    QVector<std::weak_ptr<Player>> getCollidingPlayer(const Hitbox& hitbox);
    std::weak_ptr<Player> getPlayer(unsigned number);
    uint getDefaultLightingLevel();
    double getGravity();
    void initLevelChange(ExitType e = NEXT_NORMAL);
    unsigned long getFrame();

#ifdef CARROT_DEBUG
    DebugConfig getDebugConfig();
#endif

    // TODO: phase out with better API functions
    std::weak_ptr<TileMap> getGameTiles();
    std::weak_ptr<EventMap> getGameEvents();
    SoundSystem* getSoundSystem();
    std::shared_ptr<BitmapFont> getFont();
    std::shared_ptr<ResourceSet> loadActorTypeResources(const QString& classId);
    sf::Vector2u getCanvasSize();

private:
    CarrotQt5* const mainClass;
    LevelManager* const levelManager;
};