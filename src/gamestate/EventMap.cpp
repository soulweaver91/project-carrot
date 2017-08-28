#include "EventMap.h"
#include <algorithm>
#include <cmath>
#include <QList>
#include <QSettings>
#include "../gamestate/LevelManager.h"
#include "../actor/Player.h"
#include "EventSpawner.h"

EventMap::EventMap(LevelManager* root, const EventSpawner* const spawner, const QString& eventsPath,
                   const QSettings& configuration, unsigned int width, unsigned int height, GameDifficulty difficulty)
    : root(root), spawner(spawner) {
    eventLayout = QVector<QVector<std::shared_ptr<EventTile>>>(height, QVector<std::shared_ptr<EventTile>>(width, nullptr));
    readEvents(eventsPath, configuration.value("Version/LayerFormat", 1).toUInt(), difficulty);
}

EventMap::~EventMap() {

}

bool EventMap::isPositionHurting(const CoordinatePair& pos) const {
    return getPositionEvent(pos) == PC_MODIFIER_HURT;
}

void EventMap::storeTileEvent(const TileCoordinatePair& tilePos, PCEvent e, int, const QVector<quint16>& params) {
    if (e == PC_EMPTY && (tilePos.x < 0 || tilePos.y < 0 || tilePos.y >= eventLayout.size() ||
        tilePos.x >= eventLayout[0].size() || eventLayout.at(tilePos.y).at(tilePos.x) == nullptr)) {
        return;
    }

    auto tile = std::make_shared<EventTile>();
    tile->storedEvent = e;
    tile->isEventActive = false;

    // Store event parameters
    int i = 0;
    for (; i < std::min(params.size(), 8); ++i) {
        tile->eventParams[i] = params.at(i);
    }
    for (; i < 8; ++i) {
        tile->eventParams[i] = 0;
    }

    eventLayout[tilePos.y][tilePos.x] = tile;
}

void EventMap::activateEvents(const CoordinatePair& center, int tileDistance) {
    auto tiles = root->getGameTiles().lock();
    if (tiles == nullptr) {
        return;
    }
    int x1 = std::max(0, center.tileX() - tileDistance);
    int x2 = std::min(static_cast<int>(tiles->getLevelWidth()) - 1, center.tileX() + tileDistance);
    int y1 = std::max(0, center.tileY() - tileDistance);
    int y2 = std::min(static_cast<int>(tiles->getLevelHeight()) - 1, center.tileY() + tileDistance);

    for (int x = x1; x <= x2; ++x) {
        for (int y = y1; y <= y2; ++y) {
            auto tile = eventLayout.at(y).at(x);
            if (tile == nullptr) {
                continue;
            }

            if (!tile->isEventActive && tile->storedEvent != PC_EMPTY) {
                auto ev = spawner->spawnEvent(true, tile->storedEvent, { TILE_WIDTH * (x + 0.5), TILE_HEIGHT * (y + 0.5) }, tile->eventParams);
                if (ev != nullptr) {
                    root->addActor(ev);
                }

                tile->isEventActive = true;
            }
        }
    }
}

void EventMap::deactivate(const TileCoordinatePair& tilePos) {
    if (tileHasEvent(tilePos)) {
        eventLayout[tilePos.y][tilePos.x]->isEventActive = false;
    }
}

void EventMap::deactivateAll() {
    for (int i = 0; i < eventLayout.size(); ++i) {
        for (int j = 0; j < eventLayout[i].size(); ++j) {
            deactivate(TileCoordinatePair(j, i));
        }
    }
}

PCEvent EventMap::getPositionEvent(const CoordinatePair& pos) const {
    if (tileHasEvent(pos.tilePosition())) {
        return eventLayout.at(pos.tileY()).at(pos.tileX())->storedEvent;
    }
    return PC_EMPTY;
}

void EventMap::getPositionParams(const CoordinatePair& pos, quint16 (&params)[8]) const {
    return getTileParams(pos.tilePosition(), params);
}

void EventMap::getTileParams(const TileCoordinatePair& tilePos, quint16 (&params)[8]) const {
    if (tileHasEvent(tilePos)) {
        for (int i = 0; i < 8; ++i) {
            params[i] = eventLayout.at(tilePos.y).at(tilePos.x)->eventParams[i];
        }
        return;
    }

    std::fill_n(params, 8, 0);
}

void EventMap::setTileParam(const TileCoordinatePair& tilePos, unsigned char idx, quint16 value) {
    if (idx >= 8 || !tileHasEvent(tilePos)) {
        return;
    }
    eventLayout[tilePos.y][tilePos.x]->eventParams[idx] = value;
}

void EventMap::addPlayerSpawnPoint(PlayerCharacter character, const CoordinatePair& position) {
    if (!spawnPoints.contains(character)) {
        spawnPoints.insert(character, {});
    }
    spawnPoints[character].spawnPoints << position;
}

CoordinatePair EventMap::getPlayerSpawnPoint(PlayerCharacter character) {
    if (spawnPoints.contains(character)) {
        auto& point = spawnPoints[character];

        if (point.spawnPoints.size() > 0) {
            if (point.spawnPoints.size() < point.lastUsedPoint) {
                auto coordinate = point.spawnPoints[point.lastUsedPoint];
                point.lastUsedPoint++;
                return coordinate;
            } else {
                point.lastUsedPoint = 0;
                return point.spawnPoints[0];
            }
        }
    }

    return { 8.0 * TILE_WIDTH, 1.0 * TILE_HEIGHT };
}

void EventMap::readEvents(const QString& filename, unsigned layoutVersion, GameDifficulty difficulty) {
    QFile eventMapHandle(filename);
    if (!eventMapHandle.open(QIODevice::ReadOnly)) {
        // TODO: opening failed for some reason
        return;
    }

    QByteArray eventMapData = qUncompress(eventMapHandle.readAll());
    eventMapHandle.close();

    if (eventMapData.size() == 0) {
        // TODO: uncompress fail, what do?
        return;
    }

    quint8 difficultyByte;
    switch (difficulty) {
        case DIFFICULTY_EASY:
            difficultyByte = 1;
            break;
        case DIFFICULTY_HARD:
            difficultyByte = 3;
            break;
        case DIFFICULTY_NORMAL:
        default:
            difficultyByte = 2; break;
    }

    QDataStream eventMapStream(eventMapData);
    QSet<PCEvent> encounteredEvents;
    unsigned y = 0;
    while (!eventMapStream.atEnd()) {
        unsigned x = 0;
        while (!eventMapStream.atEnd()) {
            // Read byte pairs associated with this tile
            quint16 eventID;
            eventMapStream >> eventID;
            if (eventID == 0xFFFF) {
                break;
            }
            // Initialised to have flags for easy, normal and hard difficulties
            quint8 eventFlags = 0x0E;
            QVector<quint16> eventParams(8);
            if (layoutVersion > 3) {
                eventMapStream >> eventFlags;
                for (int i = 0; i < 8; ++i) {
                    quint16 j;
                    eventMapStream >> j;
                    eventParams[i] = j;
                }
            }

            // If the difficulty bytes for the event don't match the selected difficulty, don't add anything to the event map.
            // Additionally, never show events that are multiplayer-only for now.
            if (eventFlags == 0 || ((eventFlags & (0x01 << difficultyByte)) != 0 && ((eventFlags & 0x10) == 0))) {
                encounteredEvents << (PCEvent)eventID;

                auto tilePos = TileCoordinatePair(x, y);

                switch (eventID) {
                    case PC_EMPTY:
                        break;
                    case PC_JAZZ_LEVEL_START:
                        addPlayerSpawnPoint(CHAR_JAZZ, { TILE_WIDTH * (x + 0.5), TILE_HEIGHT * (y + 0.5) });
                        break;
                    case PC_SPAZ_LEVEL_START:
                        addPlayerSpawnPoint(CHAR_SPAZ, { TILE_WIDTH * (x + 0.5), TILE_HEIGHT * (y + 0.5) });
                        break;
                    case PC_MODIFIER_ONE_WAY:
                    case PC_MODIFIER_VINE:
                    case PC_MODIFIER_HOOK:
                    case PC_MODIFIER_HURT:
                    case PC_SCENERY_DESTRUCT:
                    case PC_SCENERY_BUTTSTOMP:
                    case PC_TRIGGER_AREA:
                    case PC_SCENERY_DESTRUCT_SPD:
                    case PC_SCENERY_COLLAPSE:
                    case PC_MODIFIER_H_POLE:
                    case PC_MODIFIER_V_POLE:
                        {
                            storeTileEvent(tilePos, static_cast<PCEvent>(eventID), eventFlags, eventParams);
                            auto tiles = root->getGameTiles().lock();
                            if (tiles != nullptr) {
                                tiles->setTileEventFlag(this, tilePos, static_cast<PCEvent>(eventID));
                            }
                        }
                        break;
                    case PC_WARP_TARGET:
                        addWarpTarget(eventParams.at(0), tilePos);
                        break;
                    case PC_LIGHT_RESET:
                        eventParams[0] = root->getDefaultLightingLevel();
                        storeTileEvent(tilePos, PC_LIGHT_SET, eventFlags, eventParams);
                        break;
                    default:
                        storeTileEvent(tilePos, static_cast<PCEvent>(eventID), eventFlags, eventParams);
                        break;
                }
            }
            x++;
        }
        y++;
    }

    for (auto event : encounteredEvents) {
        resourceNames << spawner->getEventResourceName(event);
    }
}

void EventMap::addWarpTarget(unsigned id, const TileCoordinatePair& tilePos) {
    warpTargets.insert(id, tilePos);
}

bool EventMap::tileHasEvent(const TileCoordinatePair& tilePos) const {
    return (tilePos.x >= 0 && tilePos.y >= 0 && tilePos.y < eventLayout.size() && tilePos.x < eventLayout[0].size() && eventLayout.at(tilePos.y).at(tilePos.x) != nullptr);
}

CoordinatePair EventMap::getWarpTarget(unsigned id) const {
    QList<TileCoordinatePair> targets = warpTargets.values(id);
    if (targets.size() > 0) {
        auto pos = targets.at(qrand() % targets.size());
        return { pos.x * TILE_WIDTH - (TILE_WIDTH / 2.0), (pos.y + 1) * TILE_HEIGHT - 24.0 };
    } else {
        return { -1.0, -1.0 };
    }
}

QSet<QString> EventMap::getResourceNameList() const {
    return resourceNames;
}
