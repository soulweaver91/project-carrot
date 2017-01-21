#include "EventMap.h"
#include <algorithm>
#include <cmath>
#include <QList>
#include "../gamestate/LevelManager.h"
#include "../actor/Player.h"
#include "EventSpawner.h"

EventMap::EventMap(LevelManager* root, const EventSpawner* const spawner, unsigned int width, unsigned int height)
    : root(root), spawner(spawner) {
    eventLayout = QVector<QVector<std::shared_ptr<EventTile>>>(height, QVector<std::shared_ptr<EventTile>>(width, nullptr));
}

EventMap::~EventMap() {

}

bool EventMap::isPosHurting(double x, double y) {
    int ax = static_cast<int>(x) / 32;
    int ay = static_cast<int>(y) / 32;
    return getPositionEvent(ax, ay) == PC_MODIFIER_HURT;
}

unsigned short EventMap::isPosPole(double x, double y) {
    int ax = static_cast<int>(x) / 32;
    int ay = static_cast<int>(y) / 32;
    PCEvent event = getPositionEvent(ax, ay);
    return (event == PC_MODIFIER_H_POLE ? 2 :
           (event == PC_MODIFIER_V_POLE ? 1 : 0));
}

void EventMap::storeTileEvent(int x, int y, PCEvent e, int, const QVector<quint16>& params) {
    if (e == PC_EMPTY && (x < 0 || y < 0 || y >= eventLayout.size() ||
        x >= eventLayout[0].size() || eventLayout.at(y).at(x) == nullptr)) {
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

    eventLayout[y][x] = tile;
}

void EventMap::activateEvents(const CoordinatePair& center, int tileDistance) {
    auto tiles = root->getGameTiles().lock();
    if (tiles == nullptr) {
        return;
    }
    int x1 = std::max(0, static_cast<int>(center.x) / 32 - tileDistance);
    int x2 = std::min(static_cast<int>(tiles->getLevelWidth()) - 1, static_cast<int>(center.x) / 32 + tileDistance);
    int y1 = std::max(0, static_cast<int>(center.y) / 32 - tileDistance);
    int y2 = std::min(static_cast<int>(tiles->getLevelHeight()) - 1, static_cast<int>(center.y) / 32 + tileDistance);

    for (int x = x1; x <= x2; ++x) {
        for (int y = y1; y <= y2; ++y) {
            auto tile = eventLayout.at(y).at(x);
            if (tile == nullptr) {
                continue;
            }

            if (!tile->isEventActive && tile->storedEvent != PC_EMPTY) {
                auto ev = spawner->spawnEvent(tile->storedEvent, x, y, tile->eventParams);
                if (ev != nullptr) {
                    root->addActor(ev);
                }

                tile->isEventActive = true;
            }
        }
    }
}

void EventMap::deactivate(int x, int y) {
    if (positionHasEvent(x, y)) {
        eventLayout[y][x]->isEventActive = false;
    }
}

void EventMap::deactivateAll() {
    for (int i = 0; i < eventLayout.size(); ++i) {
        for (int j = 0; j < eventLayout[i].size(); ++j) {
            deactivate(j, i);
        }
    }
}

int EventMap::getPositionWarp(double x, double y) {
    int tx = static_cast<int>(x) / 32;
    int ty = static_cast<int>(y) / 32;
    if (getPositionEvent(tx, ty) == PC_WARP_ORIGIN) {
        // .get() should not fail here, as getPositionEvent() only returns PC_EVENT if coordinates had nullptr
        return eventLayout.at(ty).at(tx).get()->eventParams[0];
    } else {
        return -1;
    }
}

PCEvent EventMap::getPositionEvent(double x, double y) {
    int tx = static_cast<int>(x) / 32;
    int ty = static_cast<int>(y) / 32;
    return getPositionEvent(tx, ty);
}

PCEvent EventMap::getPositionEvent(int x, int y) {
    if (positionHasEvent(x, y)) {
        return eventLayout.at(y).at(x)->storedEvent;
    }
    return PC_EMPTY;
}

void EventMap::getPositionParams(double x, double y, quint16 (&params)[8]) {
    int tx = static_cast<int>(x) / 32;
    int ty = static_cast<int>(y) / 32;
    return getPositionParams(tx, ty, params);
}

void EventMap::getPositionParams(int x, int y, quint16 (&params)[8]) {
    if (positionHasEvent(x, y)) {
        for (int i = 0; i < 8; ++i) {
            params[i] = eventLayout.at(y).at(x)->eventParams[i];
        }
        return;
    }

    std::fill_n(params, 8, 0);
}

void EventMap::setTileParam(int x, int y, unsigned char idx, quint16 value) {
    if (idx >= 8 || !positionHasEvent(x, y)) {
        return;
    }
    eventLayout[y][x]->eventParams[idx] = value;
}

void EventMap::readEvents(const QString& filename, unsigned layoutVersion) {
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
            quint8 eventFlags = 0;
            QVector<quint16> eventParams(8);

            encounteredEvents << (PCEvent)eventID;

            if (layoutVersion > 3) {
                eventMapStream >> eventFlags;
                for (int i = 0; i < 8; ++i) {
                    quint16 j;
                    eventMapStream >> j;
                    eventParams[i] = j;
                }
            }

            switch (eventID) {
                case PC_EMPTY:
                    break;
                case PC_JAZZ_LEVEL_START:
                    if (root->getPlayer(0).lock() == nullptr) {
                        auto defaultplayer = std::make_shared<Player>(ActorInstantiationDetails(root->getActorAPI(), { 32.0 * x + 16.0, 32.0 * y + 16.0 }));
                        root->addPlayer(defaultplayer, 0);
                    }
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
                        storeTileEvent(x, y, static_cast<PCEvent>(eventID), eventFlags, eventParams);
                        auto tiles = root->getGameTiles().lock();
                        if (tiles != nullptr) {
                            tiles->setTileEventFlag(x, y, static_cast<PCEvent>(eventID));
                        }
                    }
                    break;
                case PC_WARP_TARGET:
                    addWarpTarget(eventParams.at(0), x, y);
                    break;
                case PC_LIGHT_RESET:
                    eventParams[0] = root->getDefaultLightingLevel();
                    storeTileEvent(x, y, PC_LIGHT_SET, eventFlags, eventParams);
                    break;
                default:
                    storeTileEvent(x, y, static_cast<PCEvent>(eventID), eventFlags, eventParams);
                    break;
            }
            x++;
        }
        y++;
    }

    for (auto event : encounteredEvents) {
        resourceNames << spawner->getEventResourceName(event);
    }
}

void EventMap::addWarpTarget(unsigned id, unsigned x, unsigned y) {
    CoordinatePair p = {x * 32.0 + 16.0, y * 32.0 + 30.0};
    warpTargets.insert(id, p);
}

bool EventMap::positionHasEvent(int x, int y) {
    return (x >= 0 && y >= 0 && y < eventLayout.size() && x < eventLayout[0].size() && eventLayout.at(y).at(x) != nullptr);
}

CoordinatePair EventMap::getWarpTarget(unsigned id) {
    QList<CoordinatePair> targets = warpTargets.values(id);
    if (targets.size() > 0) {
        return targets.at(qrand() % targets.size());
    } else {
        return { -1.0, -1.0 };
    }
}

QSet<QString> EventMap::getResourceNameList() {
    return resourceNames;
}
