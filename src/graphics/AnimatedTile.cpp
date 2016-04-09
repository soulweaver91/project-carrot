#include "AnimatedTile.h"

AnimatedTile::AnimatedTile(std::shared_ptr<sf::Texture> tiles_tex, const QVector<unsigned short>& tileIDs, int fps, int delay,
    int delayJitter, bool pingPong, int pingPongDelay)
    : delay(delay), delayJitter(delayJitter), pingPong(pingPong), pingPongDelay(pingPongDelay), currentTileIdx(0),
    forwards(true), framesLeft(0), frameDuration(0.0), framesRemainder(0.0), fps(fps) {
    for (unsigned tidx : tileIDs) {
        auto pseudotile = std::make_shared<LayerTile>();
        pseudotile->isAnimated = false;
        pseudotile->isFlippedX = false;
        pseudotile->isFlippedY = false;
        pseudotile->isOneWay = false;
        pseudotile->texture = tiles_tex;

        auto sprite = std::make_shared<sf::Sprite>(*tiles_tex);
        sprite->setTextureRect(sf::IntRect((tidx % 10) * 32, (tidx / 10) * 32, 32, 32));
        pseudotile->sprite = sprite;
        pseudotile->isVine = false;
        pseudotile->tileId = tidx;
        animationTiles << pseudotile;
    }
    if (fps > 0) {
        frameDuration = 70.0 / fps;
        scheduleUpdate(frameDuration);
    }
}

AnimatedTile::~AnimatedTile() {

}

std::shared_ptr<LayerTile> AnimatedTile::getCurrentTile() {
    return animationTiles.at(currentTileIdx);
}

void AnimatedTile::updateTile() {
    if (forwards) {
        if (currentTileIdx == (animationTiles.size() - 1)) {
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
