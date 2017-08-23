#pragma once

#include <memory>
#include <QSettings>
#include <QBitArray>
#include <QVector>
#include <QMultiMap>
#include <QString>
#include <SFML/Graphics.hpp>

#include "../gamestate/TileMap.h"
#include "../struct/PCEvent.h"
#include "../struct/CoordinatePair.h"
#include "../struct/GameDifficulty.h"

class CarrotQt5;
class EventSpawner;

struct EventTile {
    PCEvent storedEvent;
    bool isEventActive;
    quint16 eventParams[8];
};

class EventMap {
public:
    EventMap(LevelManager* root, const EventSpawner* const spawner, const QString& eventsPath,
             const QSettings& configuration, unsigned int width, unsigned int height, GameDifficulty difficulty = DIFFICULTY_NORMAL);
    ~EventMap();
    void storeTileEvent(const TileCoordinatePair& tilePos, PCEvent e = PC_EMPTY, int flags = 0, const QVector<quint16>& params = QVector<quint16>());
    void activateEvents(const CoordinatePair& center, int tileDistance = 32);
    PCEvent getPositionEvent(const CoordinatePair& pos) const;
    void getPositionParams(const CoordinatePair& pos, quint16 (&params)[8]) const;
    bool isPositionHurting(const CoordinatePair& pos) const;

    void deactivate(const TileCoordinatePair& tilePos);
    void deactivateAll();

    void getTileParams(const TileCoordinatePair& tilePos, quint16 (&params)[8]) const;
    void setTileParam(const TileCoordinatePair& tilePos, unsigned char idx, quint16 value);

    CoordinatePair getWarpTarget(unsigned id) const;
    QSet<QString> getResourceNameList() const;

private:
    void readEvents(const QString& filename, unsigned layoutVersion, GameDifficulty difficulty = DIFFICULTY_NORMAL);
    void addWarpTarget(unsigned id, const TileCoordinatePair& tilePos);
    bool tileHasEvent(const TileCoordinatePair& tilePos) const;
    LevelManager* root;
    QVector<QVector<std::shared_ptr<EventTile>>> eventLayout;
    QMultiMap<unsigned, TileCoordinatePair> warpTargets;
    const EventSpawner* const spawner;
    QSet<QString> resourceNames;
};
