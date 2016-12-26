#pragma once

#include <memory>
#include <QObject>
#include <QVector>
#include <SFML/Graphics.hpp>

#include "../gamestate/TileMap.h"

class AnimatedTile {
public:
    AnimatedTile(std::shared_ptr<sf::Texture> tilesTexture, const QVector<unsigned short>& tileIDs,
        const QVector<quint8>& tileFlags, int fps = 10, int delay = 0, int delayJitter = 0,
        bool pingPong = false, int pingPongDelay = 0);
    ~AnimatedTile();
    std::shared_ptr<LayerTile> getCurrentTile();
    unsigned getFrameCanonicalIndex(unsigned idx);
    unsigned getAnimationLength();
    void advanceTimer();

private:
    void scheduleUpdate(double framesToNext);
    void updateTile();

    QVector<std::shared_ptr<LayerTile>> animationTiles;
    int fps;
    int delay;
    int delayJitter;
    bool pingPong;
    int pingPongDelay;
    unsigned currentTileIdx;
    bool forwards;
        
    // current state
    unsigned long framesLeft;
    double framesRemainder;
    // initial state

    double frameDuration;
};
