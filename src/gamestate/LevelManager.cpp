#include "LevelManager.h"

#include "../CarrotQt5.h"
#include "../struct/Constants.h"
#include "../struct/Resources.h"
#include "../struct/NextLevelData.h"
#include "../actor/Player.h"
#include "GameView.h"
#include "TileMap.h"
#include "EventMap.h"
#include "ActorAPI.h"
#include "../sound/SoundSystem.h"
#include "../actor/LightSource.h"

#include <QDir>
#include <QMessageBox>
#include <QInputDialog>
#include <QStringList>
#include <QSettings>
#include <exception>

LevelManager::LevelManager(CarrotQt5* root, const QString& level, const QString& episode) :
    root(root), levelName(level), episodeName(episode), nextLevel(""), exiting(false), defaultLightingLevel(100), gravity(0.3) {

    // Fill the player pointer table with zeroes
    std::fill_n(players, 32, nullptr);

    QDir levelDir = QDir::current();
    if (episode != "") {
        levelDir = QDir(levelDir.relativeFilePath("Episodes/" + episode + "/" + level));
    } else {
        levelDir = QDir(levelDir.relativeFilePath("Levels/" + level));
    }

    if (!levelDir.exists()) {
        throw std::runtime_error(("Level folder '" + levelDir.path() + "' doesn't exist!").toStdString());
    }

    QStringList levelFiles = levelDir.entryList(QStringList("*.layer") << "config.ini");
    if (!levelFiles.contains("spr.layer") || !levelFiles.contains("config.ini")) {
        throw std::runtime_error(std::string("Sprite layer file (spr.layer) or configuration file (config.dat) missing!"));
    }

    QSettings level_config(levelDir.absoluteFilePath("config.ini"), QSettings::IniFormat);

    if (level_config.value("Version/LayerFormat", 1).toUInt() > LAYERFORMATVERSION) {
        throw std::runtime_error(std::string("The level is using a too recent layer format. You might need to update to a newer game version."));
    }

    api = std::make_shared<ActorAPI>(root, this);

    setLevelName(level_config.value("Level/FormalName", "Unnamed level").toString());

    // Show loading screen
    sf::Texture loadingScreenTexture;
    loadingScreenTexture.loadFromFile("Data/Assets/screen_loading.png");
    sf::Sprite loadingScreenSprite(loadingScreenTexture);
    loadingScreenSprite.setPosition(80, 60);
    root->getCanvas()->draw(loadingScreenSprite);
    BitmapString::drawString(root->getCanvas(), root->getFont(), levelName, 400, 360, FONT_ALIGN_CENTER);
    root->getCanvas()->updateContents();

    QString tileset = level_config.value("Level/Tileset", "").toString();

    nextLevel = level_config.value("Level/Next", "").toString();
    defaultLightingLevel = level_config.value("Level/LightInit", 100).toInt();

    QDir tileset_dir(QDir::currentPath() + QString::fromStdString("/Tilesets/") + tileset);
    if (!tileset_dir.exists()) {
        throw std::runtime_error(("Unknown tileset " + tileset).toStdString());
    }

    // Read the tileset and the sprite layer
    gameTiles = std::make_shared<TileMap>(this,
        tileset_dir.absoluteFilePath("tiles.png"),
        tileset_dir.absoluteFilePath("mask.png"),
        levelDir.absoluteFilePath("spr.layer"));

    // Read the sky layer if it exists
    if (levelFiles.contains("sky.layer")) {
        gameTiles->readLayerConfiguration(LAYER_SKY_LAYER, levelDir.absoluteFilePath("sky.layer"), level_config, 0);
    }

    // Read the background layers
    QStringList bgLayers = levelFiles.filter(".bg.layer");
    for (int i = 0; i < bgLayers.size(); ++i) {
        gameTiles->readLayerConfiguration(LAYER_BACKGROUND_LAYER, levelDir.absoluteFilePath(bgLayers.at(i)), level_config, i);
    }

    // Read the foreground layers
    QStringList fgLayers = levelFiles.filter(".fg.layer");
    for (int i = 0; i < fgLayers.size(); ++i) {
        gameTiles->readLayerConfiguration(LAYER_FOREGROUND_LAYER, levelDir.absoluteFilePath(fgLayers.at(i)), level_config, i);
    }

    if (levelDir.entryList().contains("animtiles.dat")) {
        gameTiles->readAnimatedTiles(levelDir.absoluteFilePath("animtiles.dat"));
    }

    gameEvents = std::make_shared<EventMap>(this, gameTiles->getLevelWidth(), gameTiles->getLevelHeight());
    if (levelFiles.contains("event.layer")) {
        gameEvents->readEvents(levelDir.absoluteFilePath("event.layer"), level_config.value("Version/LayerFormat", 1).toUInt());
    }

    gameTiles->saveInitialSpriteLayer();

    if (players[0] == nullptr) {
        auto defaultplayer = std::make_shared<Player>(api, 320.0, 32.0);
        addPlayer(defaultplayer, 0);
    }

    auto soundSystem = root->getSoundSystem();

    views.clear();
    sf::Vector2f viewSize(root->getCanvas()->getSize().x, root->getCanvas()->getSize().y);
    for (uint i = 0; i < 32; ++i) {
        if (players[i] != nullptr) {
            views.append(std::make_shared<GameView>(this, i, viewSize));
            players[i]->setView(views.last());

            if (soundSystem != nullptr) {
                soundSystem->registerSoundListener(players[i]->shared_from_this());
            }
        }
    }

    for (auto view : views) {
        view->setLighting(defaultLightingLevel, true);
    }

    root->getSoundSystem()->setMusic(("Music/" + level_config.value("Level/MusicDefault", "").toString().toUtf8()).data());
    setSavePoint();
}

LevelManager::~LevelManager() {
    cleanUpLevel();
}

void LevelManager::cleanUpLevel() {
    actors.clear();
    std::fill_n(players, 32, nullptr);

    views.clear();
}

void LevelManager::processControlEvents(const ControlEventList& events) {
    QVector<InteractiveActor*> interactiveActors;
    for (auto actor : actors) {
        if (auto ptr = dynamic_cast<InteractiveActor*>(actor.get())) {
            interactiveActors << ptr;
        }
    }

    for (const auto& pair : events.controlDownEvents) {
        foreach(auto& actor, interactiveActors) {
            actor->processControlDownEvent(pair);
        }
    }

    foreach(auto& actor, interactiveActors) {
        actor->processAllControlHeldEvents(events.controlHeldEvents);
    }

    for (const auto& pair : events.controlUpEvents) {
        foreach(auto& actor, interactiveActors) {
            actor->processControlUpEvent(pair);
        }
    }
}

bool LevelManager::addActor(std::shared_ptr<CommonActor> actor) {
    actors.push_back(actor);
    return true;
}

bool LevelManager::addPlayer(std::shared_ptr<Player> actor, short playerID) {
    // If player ID is defined and is between 0 and 31 (inclusive),
    // try to add the player to the player array
    if ((playerID > -1) && (playerID < 32)) {
        if (players[playerID] != nullptr) {
            // A player with that number already existed
            return false;
        }
        // all OK, add to the player 
        players[playerID] = actor;
    }
    actors.push_back(actor);
    return true;
}

void LevelManager::tick(const ControlEventList& events) {
    auto canvas = root->getCanvas();

    advanceTimers();

    // Deactivate far away instances, create near instances
    // TODO: Adapt for multiple players
    int view_x = static_cast<unsigned>(views[0]->getViewCenter().x) / 32;
    int view_y = static_cast<unsigned>(views[0]->getViewCenter().y) / 32;
    for (int i = 0; i < actors.size(); i++) {
        if (actors.at(i)->deactivate(view_x, view_y, 32)) {
            --i;
        }
    }
    gameEvents->activateEvents(views[0]->getViewCenter());

    // Run animated tiles' timers
    gameTiles->advanceAnimatedTileTimers();
    gameTiles->advanceCollapsingTileTimers();

    // Run all actors' timers
    for (int i = 0; i < actors.size(); i++) {
        actors.at(i)->advanceActorAnimationTimers();
        actors.at(i)->advanceTimers();
        actors.at(i)->updateGraphicState();
    }

    // Run all actors' tick events
    for (int i = 0; i < actors.size(); i++) {
        actors.at(i)->tickEvent();
        if (actors.at(i)->perish()) {
            --i;
        }
    }

    root->getSoundSystem()->updateSoundPositions();

    // TODO: Adapt for multiple players
    for (int i = 0; i < debris.size(); i++) {
        debris.at(i)->tickUpdate();
        if (debris.at(i)->getY() - views[0]->getViewCenter().y > 400) {
            debris.erase(debris.begin() + i);
            --i;
        }
    }

    processControlEvents(events);

    for (auto view : views) {
        // Set player to the center of the view
        view->centerToPlayer();

        // Draw the layers: first lower (background and sprite) levels...
        gameTiles->drawLowerLevels(view);

        // ...then draw all the actors...
        for (auto& actor : actors) {
            actor->drawUpdate(view);
        }
        // ...then all the debris elements...
        for (auto& oneDebris : debris) {
            oneDebris->drawUpdate(view);
        }

        // ...and finally the higher (foreground) levels
        gameTiles->drawHigherLevels(view);

        QVector<LightSource*> lightSources;
        for (auto actor : actors) {
            if (dynamic_cast<LightSource*>(actor.get()) != nullptr) {
                lightSources << dynamic_cast<LightSource*>(actor.get());
            }
        }
        view->drawBackgroundEffects(lightSources);
        view->drawLighting(lightSources);
        view->drawUiElements();
        view->drawView(root->getCanvas());
    }

#ifdef CARROT_DEBUG
    auto debugConfig = root->getDebugConfig();
    if (debugConfig.dbgOverlaysActive) {
        float fps = root->getCurrentFPS();
        //BitmapString::drawString(window,mainFont,"Frame: " + QString::number(frame),6,56);
        BitmapString::drawString(canvas, root->getFont(), "Actors: " + QString::number(actors.size()), 6, 176);
        BitmapString::drawString(canvas, root->getFont(), "FPS: " + QString::number(fps, 'f', 2) + " at " +
            QString::number(1000 / root->getCurrentFPS()) + "ms/f", 6, 56);
        BitmapString::drawString(canvas, root->getFont(), "Mod-" +
            QString::number(debugConfig.currentTempModifier) + " " +
            debugConfig.tempModifierName[debugConfig.currentTempModifier] + ": " +
            QString::number(debugConfig.tempModifier[debugConfig.currentTempModifier]), 6, 540);
    }
#endif
}

void LevelManager::removeActor(std::shared_ptr<CommonActor> actor) {
    for (int i = 0; i < actors.size(); ++i) {
        if (actors.at(i) == actor) {
            actors.erase(actors.begin() + i);
            return;
        }
    }
}

QVector<std::weak_ptr<CommonActor>> LevelManager::findCollisionActors(const Hitbox& hitbox, std::shared_ptr<CommonActor> me) {
    QVector<std::weak_ptr<CommonActor>> res;
    for (int i = 0; i < actors.size(); ++i) {
        if (me == actors.at(i)) {
            continue;
        }
        if (actors.at(i)->getHitbox().overlaps(hitbox)) {
            res << actors.at(i);
        }
    }
    return res;
}

QVector<std::weak_ptr<CommonActor>> LevelManager::findCollisionActors(std::shared_ptr<CommonActor> me) {
    QVector<std::weak_ptr<CommonActor>> res;

    if (!me->getIsCollidable()) {
        return res;
    }

    const auto& myGS = me->getGraphicState();
    QTransform tfMatrixA;
    tfMatrixA
        .rotate(myGS.angle * 360)
        .scale(myGS.scale.x, myGS.scale.y)
        .translate(-myGS.hotspot.x, -myGS.hotspot.y);

    for (auto& actor : actors) {
        if (me == actor || !actor->getIsCollidable()) {
            continue;
        }
        const auto& otherGS = actor->getGraphicState();
        bool done = false;

        if (!myGS.boundingBox.intersects(otherGS.boundingBox)) {
            continue;
        }

        QTransform tfMatrixB;
        tfMatrixB
            .translate(otherGS.origin.x - myGS.origin.x, otherGS.origin.y - myGS.origin.y)
            .rotate(otherGS.angle * 360)
            .scale(otherGS.scale.x, otherGS.scale.y)
            .translate(-otherGS.hotspot.x, -otherGS.hotspot.y);
        tfMatrixB = tfMatrixB.inverted();

        for (int y = 0; y < myGS.dimensions.y; ++y) {
            if (done) {
                break;
            }
            for (int x = 0; x < myGS.dimensions.x; ++x) {
                if (done) {
                    break;
                }

                if (!myGS.mask->at(y * myGS.dimensions.x + x)) {
                    continue;
                }

                qreal worldX;
                qreal worldY;
                tfMatrixA.map(x, y, &worldX, &worldY);

                qreal otherTexPosX;
                qreal otherTexPosY;
                tfMatrixB.map(worldX, worldY, &otherTexPosX, &otherTexPosY);
                int otherTexPosXInt = qRound(otherTexPosX);
                int otherTexPosYInt = qRound(otherTexPosY);

                if (otherTexPosXInt < 0
                    || otherTexPosYInt < 0
                    || otherTexPosXInt >= otherGS.dimensions.x
                    || otherTexPosYInt >= otherGS.dimensions.y) {
                    continue;
                }

                if (otherGS.mask->at(otherTexPosXInt + otherGS.dimensions.x * otherTexPosYInt)) {
                    done = true;
                    res << actor;
                }
            }
        }
    }
    return res;
}

void LevelManager::setSavePoint() {
    lastSavePoint.playerLives = players[0]->getLives() - 1;
    lastSavePoint.playerPosition = players[0]->getPosition();
    lastSavePoint.spriteLayerState = gameTiles->prepareSavePointLayer();
}

void LevelManager::loadSavePoint() {
    clearActors();
    gameEvents->deactivateAll();
    players[0]->moveInstantly(lastSavePoint.playerPosition, true);
    gameTiles->loadSavePointLayer(lastSavePoint.spriteLayerState);
}

void LevelManager::clearActors() {
    actors.clear();
    for (uint i = 0; i < 32; ++i) {
        if (players[i] != nullptr) {
            actors << players[i];
        }
    }
}

void LevelManager::createDebris(unsigned tileId, int x, int y) {
    for (int i = 0; i < 4; ++i) {
        auto d = std::make_shared<DestructibleDebris>(gameTiles->getTilesetTexture(), x, y, tileId % 10, tileId / 10, i);
        debris << d;
    }
}

bool LevelManager::isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me,
    std::weak_ptr<SolidObject>& collisionActor) {
    collisionActor = std::weak_ptr<SolidObject>();
    if (!gameTiles->isTileEmpty(hitbox, downwards)) {
        return false;
    }

    // check for solid objects
    QVector<std::weak_ptr<CommonActor>> collision = findCollisionActors(hitbox, me);
    for (int i = 0; i < collision.size(); ++i) {
        auto collisionPtr = collision.at(i).lock();
        if (collisionPtr == nullptr) {
            continue;
        }

        auto object = std::dynamic_pointer_cast<SolidObject>(collisionPtr);
        if (object == nullptr) {
            continue;
        }

        if (!object->getIsOneWay() || downwards) {
            collisionActor = object;
            return false;
        }
    }
    return true;
}

// alternate version to be used if we don't care what solid object we collided with
bool LevelManager::isPositionEmpty(const Hitbox& hitbox, bool downwards, std::shared_ptr<CommonActor> me) {
    std::weak_ptr<SolidObject> placeholder;
    return isPositionEmpty(hitbox, downwards, me, placeholder);
}

QVector<std::weak_ptr<Player>> LevelManager::getCollidingPlayer(const Hitbox& hitbox) {
    QVector<std::weak_ptr<Player>> result;

    for (auto p : players) {
        if (p == nullptr) {
            continue;
        }

        Hitbox pHitbox = p->getHitbox();
        if (!(
            pHitbox.right < hitbox.left || pHitbox.left > hitbox.right ||
            pHitbox.bottom < hitbox.top || pHitbox.top > hitbox.bottom
            )) {
            result << p;
        }
    }

    return result;
}

std::weak_ptr<Player> LevelManager::getPlayer(unsigned number) {
    if (number > 32) {
        return std::weak_ptr<Player>();
    } else {
        return players[number];
    }
}

std::weak_ptr<TileMap> LevelManager::getGameTiles() {
    return gameTiles;
}

std::weak_ptr<EventMap> LevelManager::getGameEvents() {
    return gameEvents;
}

uint LevelManager::getDefaultLightingLevel() {
    return defaultLightingLevel;
}

double LevelManager::getGravity() {
    return gravity;
}

void LevelManager::resizeEvent(int w, int h) {
    if (gameTiles != nullptr) {
        gameTiles->initializeTexturedBackgroundFade(w, h);
    }
    for (auto view : views) {
        view->setSize(sf::Vector2f(w, h));
    }
}

void LevelManager::setLevelName(const QString& name) {
    levelName = name;
    root->setWindowTitle("Project Carrot - " + levelName);
}

void LevelManager::initLevelChange(ExitType e) {
    if (exiting) {
        return;
    }
    exiting = true;

    root->getSoundSystem()->setMusic("");

    addTimer(435u, false, [this, e]() {
        LevelCarryOver nextLevelData = players[0]->prepareLevelCarryOver();
        nextLevelData.exitType = e;

        root->startGame(nextLevel, episodeName, nextLevelData);
    });
}

std::shared_ptr<ActorAPI> LevelManager::getActorAPI() {
    return api;
}

void LevelManager::processCarryOver(const LevelCarryOver carryOver) {
    if (carryOver.exitType != NEXT_NONE) {
        players[0]->receiveLevelCarryOver(carryOver);
    }
}

#ifdef CARROT_DEBUG

void LevelManager::debugSetGravity() {
    gravity = QInputDialog::getDouble(root, "Set new gravity", "Gravity:", gravity, -3.0, 3.0, 4);
}

void LevelManager::debugSetLighting() {
    int lightingLevel = QInputDialog::getInt(root, "Set new lighting", "Lighting:", 100, 0, 100);
    for (auto view : views) {
        view->setLighting(lightingLevel, true);
    }
}

void LevelManager::debugSetPosition() {
    auto player = getPlayer(0).lock();
    if (player == nullptr) {
        return;
    }

    int x = QInputDialog::getInt(root, "Move player", "X position", player->getPosition().x, 0, gameTiles->getLevelWidth() * 32);
    int y = QInputDialog::getInt(root, "Move player", "Y position", player->getPosition().y, 0, gameTiles->getLevelHeight() * 32);
    player->moveInstantly({ x * 1.0, y * 1.0 }, true);
}

void LevelManager::debugSugarRush() {
    auto player = getPlayer(0).lock();
    if (player == nullptr) {
        return;
    }

    for (int i = 0; i < 100; ++i) {
        player->consumeFood(false);
    }
}

#endif
