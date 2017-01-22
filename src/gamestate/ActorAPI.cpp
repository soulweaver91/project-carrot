#include "ActorAPI.h"
#include "EventSpawner.h"

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

std::shared_ptr<CommonActor> ActorAPI::createActor(PCEvent type, double x, double y, const quint16 params[8], bool returnOnly) {
    auto ev = mainClass->getEventSpawner()->spawnEvent(false, type, x / 32, y / 32, params);
    if (!returnOnly) {
        addActor(ev);
    }

    return ev;
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

std::shared_ptr<BitmapString> ActorAPI::makeString(const QString& initString, BitmapFontSize size, FontAlign initAlign) {
    return std::make_shared<BitmapString>(mainClass->getFont(size), initString, initAlign);
}

uint ActorAPI::getStringWidth(const QString& text, BitmapFontSize size) {
    return BitmapString(mainClass->getFont(size), text).getWidth();
}

QString ActorAPI::getLevelText(int idx) {
    return levelManager->getLevelText(idx);
}

void ActorAPI::handleGameOver() {
    levelManager->handleGameOver();
}

#ifdef CARROT_DEBUG
DebugConfig* ActorAPI::getDebugConfig() {
    return mainClass->getDebugConfig();
}
#endif

std::weak_ptr<TileMap> ActorAPI::getGameTiles() {
    return levelManager->getGameTiles();
}

std::weak_ptr<EventMap> ActorAPI::getGameEvents() {
    return levelManager->getGameEvents();
}

std::shared_ptr<ResourceSet> ActorAPI::loadActorTypeResources(const QString& classId) {
    return mainClass->loadActorTypeResources(classId);
}

template <typename... P>
void ActorAPI::playSound(HSAMPLE sample, P... params) {
    mainClass->getSoundSystem()->playSFX(sample, params...);
}

// Specify allowed template instantations
template void ActorAPI::playSound(HSAMPLE, CoordinatePair);
template void ActorAPI::playSound(HSAMPLE, CoordinatePair, float);
template void ActorAPI::playSound(HSAMPLE, CoordinatePair, float, float);
template void ActorAPI::playSound(HSAMPLE, CoordinatePair, float, float, float);
template void ActorAPI::playSound(HSAMPLE, bool);
template void ActorAPI::playSound(HSAMPLE, bool, float);
template void ActorAPI::playSound(HSAMPLE, bool, float, float);
template void ActorAPI::playSound(HSAMPLE, bool, float, float, float);

void ActorAPI::pauseMusic() {
    mainClass->getSoundSystem()->pauseMusic();
}

void ActorAPI::resumeMusic() {
    mainClass->getSoundSystem()->resumeMusic();
}
