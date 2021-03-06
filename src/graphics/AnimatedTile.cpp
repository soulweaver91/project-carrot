#include "AnimatedTile.h"
#include <cmath>

AnimatedTile::AnimatedTile(std::shared_ptr<sf::Texture> tiles_tex, const QVector<quint16>& tileIDs,
    const QVector<quint8>& tileFlags, int fps, int delay, int delayJitter, bool pingPong, int pingPongDelay)
    : fps(fps), delay(delay), delayJitter(delayJitter), pingPong(pingPong), pingPongDelay(pingPongDelay), currentTileIdx(0),
    forwards(true), framesLeft(0), framesRemainder(0.0), frameDuration(0.0) {
    quint8 idx = 0;
    for (unsigned tidx : tileIDs) {
        auto pseudotile = std::make_shared<LayerTile>();
        pseudotile->isAnimated = false;
        pseudotile->isFlippedX = false;
        pseudotile->isFlippedY = false;
        pseudotile->isOneWay = false;
        pseudotile->texture = tiles_tex;

        auto sprite = std::make_shared<sf::Sprite>(*tiles_tex);
        sprite->setTextureRect(sf::IntRect((tidx % 10) * TILE_WIDTH, (tidx / 10) * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));

        if (tileFlags.size() > idx && ((tileFlags.at(idx) & 0x80) > 0)) {
            sprite->setColor(sf::Color(255, 255, 255, 127));
        }

        pseudotile->sprite = sprite;
        pseudotile->suspendType = SuspendType::SUSPEND_NONE;
        pseudotile->tileId = tidx;
        animationTiles << pseudotile;
        idx++;
    }
    if (fps > 0) {
        frameDuration = 70.0 / fps;
        scheduleUpdate(frameDuration);
    }

    // Ignore unused member warning
    (void)this->delayJitter;
}

AnimatedTile::~AnimatedTile() {

}

std::shared_ptr<LayerTile> AnimatedTile::getCurrentTile() {
    return animationTiles.at(currentTileIdx);
}

void AnimatedTile::updateTile() {
    if (animationTiles.size() < 2) {
        return;
    }

    if (forwards) {
        if (currentTileIdx == static_cast<uint>(animationTiles.size() - 1)) {
            if (pingPong) {
                forwards = false;
                scheduleUpdate(frameDuration * (1 + pingPongDelay));
            } else {
                currentTileIdx = 0;
                scheduleUpdate(frameDuration * (1 + delay));
            }
        } else {
            currentTileIdx += 1;
            scheduleUpdate(frameDuration);
        }
    } else {
        if (currentTileIdx == 0) {
            // reverse only occurs on ping pong mode so no need to check for that here
            forwards = true;
            scheduleUpdate(frameDuration * (1 + delay));
        } else {
            currentTileIdx -= 1;
            scheduleUpdate(frameDuration);
        }
    }
}

unsigned AnimatedTile::getFrameCanonicalIndex(unsigned idx) {
    return animationTiles.at(idx)->tileId;
}

unsigned AnimatedTile::getAnimationLength() {
    return animationTiles.size();
}

void AnimatedTile::scheduleUpdate(double framesToNext) {
    int framesToNextInt = qRound(std::floor(framesToNext));
    double framesToNextRemainder = framesToNext - framesToNextInt;

    framesLeft += framesToNextInt;
    framesRemainder += framesToNextRemainder;
    while (framesRemainder > 1) {
        framesLeft++;
        framesRemainder -= 1;
    }
}

void AnimatedTile::advanceTimer() {
    if (fps == 0) {
        return;
    }
    if (framesLeft > 0) {
        framesLeft--;
    } else {
        updateTile();
    }
}
