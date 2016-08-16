#include "ActorAPI.h"

ActorAPI::ActorAPI(CarrotQt5* mainClass, LevelManager* levelManager) 
    : mainClass(mainClass), levelManager(levelManager) {
}

ActorAPI::~ActorAPI() {
}

bool ActorAPI::addActor(std::shared_ptr<CommonActor> actor) {
    return levelManager->addActor(actor);
}

void ActorAPI::removeActor(std::shared_ptr<CommonActor> actor) {
    return levelManager->removeActor(actor);
}

QVector<std::weak_ptr<CommonActor>> ActorAPI::findCollisionActors(const Hitbox& hitbox, std::shared_ptr<CommonActor> me) {
    return levelManager->findCollisionActors(hitbox, me);
}

QVector<std::weak_ptr<CommonActor>> ActorAPI::findCollisionActors(std::shared_ptr<CommonActor> me) {
    return levelManager->findCollisionActors(me);
}

void ActorAPI::setSavePoint() {
    return levelManager->setSavePoint();
}

void ActorAPI::loadSavePoint() {
    return levelManager->loadSavePoint();
}

bool ActorAPI::isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me, std::weak_ptr<SolidObject>& collisionActor) {
    return levelManager->isPositionEmpty(hitbox, downwards, me, collisionActor);
}

bool ActorAPI::isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me) {
    return levelManager->isPositionEmpty(hitbox, downwards, me);
}

QVector<std::weak_ptr<Player>> ActorAPI::getCollidingPlayer(const Hitbox& hitbox) {
    return levelManager->getCollidingPlayer(hitbox);
}

std::weak_ptr<Player> ActorAPI::getPlayer(unsigned number) {
    return levelManager->getPlayer(number);
}

uint ActorAPI::getDefaultLightingLevel() {
    return levelManager->getDefaultLightingLevel();
}

double ActorAPI::getGravity() {
    return levelManager->getGravity();
}

void ActorAPI::initLevelChange(ExitType e) {
    return levelManager->initLevelChange(e);
}

unsigned long ActorAPI::getFrame() {
    return mainClass->getFrame();
}

#ifdef CARROT_DEBUG
DebugConfig ActorAPI::getDebugConfig() {
    return mainClass->getDebugConfig();
}
#endif

std::weak_ptr<TileMap> ActorAPI::getGameTiles() {
    return levelManager->getGameTiles();
}

std::weak_ptr<EventMap> ActorAPI::getGameEvents() {
    return levelManager->getGameEvents();
}

SoundSystem* ActorAPI::getSoundSystem() {
    return mainClass->getSoundSystem();
}

std::shared_ptr<BitmapFont> ActorAPI::getFont() {
    return mainClass->getFont();
}

std::shared_ptr<ResourceSet> ActorAPI::loadActorTypeResources(const QString& classId) {
    return mainClass->loadActorTypeResources(classId);
}

sf::Vector2u ActorAPI::getCanvasSize() {
    return mainClass->getCanvas()->getSize();
}

