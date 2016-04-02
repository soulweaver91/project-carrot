#pragma once

#include <QObject>
#include <QList>
#include <SFML/Graphics.hpp>

#include "../gamestate/TileMap.h"

class AnimatedTile {
    public:
        AnimatedTile(TileMap* ptr, const QList< unsigned short >& tile_refs, int fps = 10, int delay = 0, 
            int delay_jitter = 0, bool ping_pong = false, int ping_pong_delay = 0);
        ~AnimatedTile();
        LayerTile* getCurrentTile();
        unsigned getFrameCanonicalIndex(unsigned idx);
        unsigned getAnimationLength();
        void advanceTimer();

    private:
        void scheduleUpdate(double frames_to_next);
        void updateTile();

        QList< LayerTile* > animation_tiles;
        int fps;
        int delay;
        int delay_jitter;
        bool ping_pong;
        int ping_pong_delay;
        unsigned curr_idx;
        bool forwards;
        
        // current state
        unsigned long frames_left;
        double frames_remainder;
        // initial state
        double frames_original;
};
