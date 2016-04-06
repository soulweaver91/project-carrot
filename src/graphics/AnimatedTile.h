#pragma once

#include <memory>
#include <QObject>
#include <QVector>
#include <SFML/Graphics.hpp>

#include "../gamestate/TileMap.h"

class AnimatedTile {
    public:
        AnimatedTile(std::shared_ptr<sf::Texture> tiles_tex, const QVector<unsigned short>& tile_refs,
            int fps = 10, int delay = 0, int delay_jitter = 0, bool ping_pong = false, 
            int ping_pong_delay = 0);
        ~AnimatedTile();
        std::shared_ptr<LayerTile> getCurrentTile();
        unsigned getFrameCanonicalIndex(unsigned idx);
        unsigned getAnimationLength();
        void advanceTimer();

    private:
        void scheduleUpdate(double frames_to_next);
        void updateTile();

        QVector<std::shared_ptr<LayerTile>> animation_tiles;
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
