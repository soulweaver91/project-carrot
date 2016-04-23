#include "EventMap.h"
#include <QList>
#include "../CarrotQt5.h"
#include "../actor/SolidObject.h"
#include "../actor/Collectible.h"
#include "../actor/Player.h"
#include "../actor/enemy/Enemy.h"
#include "../actor/PushBox.h"
#include "../actor/TriggerCrate.h"
#include "../actor/Bridge.h"
#include "../actor/enemy/NormalTurtle.h"
#include "../actor/enemy/Lizard.h"
#include "../actor/SavePoint.h"
#include "../actor/Spring.h"
#include "../actor/MovingPlatform.h"
#include "../struct/WeaponTypes.h"

EventMap::EventMap(std::shared_ptr<CarrotQt5> gameRoot, unsigned int width, unsigned int height)
    : root(gameRoot) {
    for (unsigned int y = 0; y <= height; ++y) {
        QVector<std::shared_ptr<EventTile>> n;
        for (unsigned int x = 0; x <= width; ++x) {
            // std::fill_n doesn't seem to work here
            n << nullptr;
        }
        eventLayout << n;
    }
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

void EventMap::storeTileEvent(int x, int y, PCEvent e, int flags, const QVector<quint16>& params) {
    if (e == PC_NONE && (x < 0 || y < 0 || y >= eventLayout.size() ||
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

void EventMap::activateEvents(const sf::View& center, int tileDistance) {
    int x1 = std::max(0, static_cast<int>(center.getCenter().x) / 32 - tileDistance);
    int x2 = std::min(static_cast<int>(root->getLevelWidth()) - 1, static_cast<int>(center.getCenter().x) / 32 + tileDistance);
    int y1 = std::max(0, static_cast<int>(center.getCenter().y) / 32 - tileDistance);
    int y2 = std::min(static_cast<int>(root->getLevelHeight()) - 1, static_cast<int>(center.getCenter().y) / 32 + tileDistance);

    for (unsigned x = x1; x <= x2; ++x) {
        for (unsigned y = y1; y <= y2; ++y) {
            auto tile = eventLayout.at(y).at(x);
            if (tile == nullptr) {
                continue;
            }

            if (!tile->isEventActive && tile->storedEvent != PC_EMPTY) {
                switch (tile->storedEvent) {
                    case PC_FAST_FIRE:
                    case PC_GEM_RED:
                    case PC_GEM_GREEN:
                    case PC_GEM_BLUE:
                    case PC_AMMO_TOASTER:
                    case PC_AMMO_BOUNCER:
                    case PC_AMMO_FREEZER:
                    case PC_AMMO_SEEKER:
                    case PC_AMMO_RF:
                    case PC_AMMO_TNT:
                    case PC_AMMO_PEPPER:
                    case PC_AMMO_ELECTRO:
                    case PC_COIN_SILVER:
                    case PC_COIN_GOLD:
                        {
                            auto c = std::make_shared<Collectible>(root, static_cast<CollectibleType>(tile->storedEvent), 
                                32.0 * x + 16.0, 32.0 * y + 16.0);
                            root->addActor(c);
                        }
                        break;
                    case PC_ENEMY_TURTLE_NORMAL:
                        {
                            auto e = std::make_shared<EnemyNormalTurtle>(root, 32.0 * x + 16.0, 32.0 * y + 16.0);
                            root->addActor(e);
                        }
                        break;
                    case PC_ENEMY_LIZARD:
                        {
                            auto e = std::make_shared<EnemyLizard>(root, 32.0 * x + 16.0, 32.0 * y + 16.0);
                            root->addActor(e);
                        }
                        break;
                    case PC_SAVE_POINT:
                        {
                            auto e = std::make_shared<SavePoint>(root, 32.0 * x + 16.0, 32.0 * y + 16.0);
                            root->addActor(e);
                        }
                        break;
                    case PC_PUSHABLE_ROCK:
                        {
                            auto e = std::make_shared<PushBox>(root, 32.0 * x + 16.0, 32.0 * y + 16.0, tile->eventParams[0]);
                            root->addActor(e);
                        }
                        break;
                    case PC_TRIGGER_CRATE:
                        {
                            auto e = std::make_shared<TriggerCrate>(root, 32.0 * x + 16.0, 32.0 * y + 16.0, tile->eventParams[0]);
                            root->addActor(e);
                        }
                        break;
                    case PC_BRIDGE:
                        {
                            auto e = std::make_shared<DynamicBridge>(root, 32.0 * x + 16.0, 32.0 * y + 16.0,
                                tile->eventParams[0], static_cast<DynamicBridgeType>(tile->eventParams[1]), tile->eventParams[2]);
                            root->addActor(e);
                        }
                        break;
                    case PC_SPRING_RED:
                    case PC_SPRING_GREEN:
                    case PC_SPRING_BLUE:
                        {
                            auto e = std::make_shared<Spring>(root, 32.0 * x + 16.0, 32.0 * y + 16.0, 
                                (SpringType)(1 + (tile->storedEvent - PC_SPRING_RED)), (byte)tile->eventParams[0]);
                            root->addActor(e);
                        }
                        break;
                    case PC_MOVING_PLATFORM:
                        {
                            auto e = std::make_shared<MovingPlatform>(root, 32.0 * x + 16.0, 32.0 * y + 16.0,
                                static_cast<PlatformType>(tile->eventParams[0]),
                                tile->eventParams[3], (qint16)tile->eventParams[2], tile->eventParams[1],
                                tile->eventParams[4] != 0);
                            root->addActor(e);
                        }
                        break;
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
    if (eventMapHandle.open(QIODevice::ReadOnly)) {
        QByteArray eventMapData = qUncompress(eventMapHandle.readAll());
        if (eventMapData.size() > 0) {
            QDataStream eventMapStream(eventMapData);
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
                                auto defaultplayer = std::make_shared<Player>(root, 32.0 * x + 16.0, 32.0 * y + 16.0);
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
                            eventParams[0] = root->getLightingLevel();
                            storeTileEvent(x, y, PC_LIGHT_SET, eventFlags, eventParams);
                        default:
                            storeTileEvent(x, y, static_cast<PCEvent>(eventID), eventFlags, eventParams);
                            break;
                    }
                    x++;
                }
                y++;
            }
        } else {
            // TODO: uncompress fail, what do?
        }
    } else {
        // TODO: opening failed for some reason
    }
    eventMapHandle.close();
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

