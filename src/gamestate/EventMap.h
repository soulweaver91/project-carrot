#pragma once

#include <memory>
#include <QSettings>
#include <QBitArray>
#include <QList>
#include <QMultiMap>
#include <QString>
#include <SFML/Graphics.hpp>

#include "../CarrotQt5.h"
#include "../gamestate/TileMap.h"

struct EventTile {
    PCEvent stored_event;
    bool event_active;
    quint16 event_params[8];
};

class EventMap {
    public:
        EventMap(std::shared_ptr<CarrotQt5> game_root, TileMap* game_tiles, unsigned int width, unsigned int height);
        ~EventMap();
        void storeTileEvent(int x, int y, PCEvent e = PC_EMPTY, int flags = 0, const QList< quint16 >& params = QList< quint16 >());
        void activateEvents(const sf::View& center, int dist_tiles = 32);
        bool isPosHurting(double x, double y);
        unsigned short isPosPole(double x, double y);
        void deactivate(int x, int y);
        int getPositionWarp(double x, double y);
        PCEvent getPositionEvent(double x, double y);
        void getPositionParams(double x, double y, quint16 (&params)[8]);
        void deactivateAll();
        void setTileParam(int x, int y, unsigned char idx, quint16 value);
        void readEvents(const QString& filename, unsigned layout_version);
        CoordinatePair getWarpTarget(unsigned id);

    private:
        void addWarpTarget(unsigned id, unsigned x, unsigned y);
        std::shared_ptr<CarrotQt5> root;
        TileMap* tiles;
        QList< QList< EventTile > > event_layout;
        QMultiMap< unsigned, CoordinatePair > warpTargets;
};
