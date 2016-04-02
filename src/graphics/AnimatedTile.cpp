#include "AnimatedTile.h"

AnimatedTile::AnimatedTile(TileMap* ptr, const QList< unsigned short >& tile_ids, int fps, int delay, int delay_jitter, bool ping_pong, int ping_pong_delay)
    : delay(delay), delay_jitter(delay_jitter), ping_pong(ping_pong), ping_pong_delay(ping_pong_delay), curr_idx(0), forwards(true), frames_left(0),
      frames_original(0.0), frames_remainder(0.0), fps(fps) {
    const sf::Texture* tiles_tex = ptr->getTilesetTexture();
    for (unsigned tidx : tile_ids) {
        LayerTile* pseudotile = new LayerTile;
        pseudotile->animated = false;
        pseudotile->flipped_x = false;
        pseudotile->flipped_y = false;
        pseudotile->oneway = false;
        sf::Sprite* spr = new sf::Sprite(*tiles_tex);
        spr->setTextureRect(sf::IntRect((tidx % 10) * 32, (tidx / 10) * 32, 32, 32));
        pseudotile->sprite = *spr;
        pseudotile->vine = false;
        pseudotile->tile_id = tidx;
        animation_tiles.push_back(pseudotile);
    }
    if (fps > 0) {
        //ani_timer = new QTimer();
        //ani_timer->setSingleShot(true);
        //connect(ani_timer,SIGNAL(timeout()),this,SLOT(updateTile()));
        //ani_timer->start(1000 / fps);

        frames_original = 70.0 / fps;
        scheduleUpdate(frames_original);
    }
}

AnimatedTile::~AnimatedTile() {

}

LayerTile* AnimatedTile::getCurrentTile() {
    return animation_tiles.at(curr_idx);
}

void AnimatedTile::updateTile() {
    if (forwards) {
        if (curr_idx == (animation_tiles.size() - 1)) {
            if (ping_pong) {
                forwards = false;
                
                //ani_timer->start(1000 / fps * (1 + ping_pong_delay));
                scheduleUpdate(frames_original * (1 + ping_pong_delay));
            } else {
                curr_idx = 0;
                //ani_timer->start(1000 / fps * (1 + delay));
                scheduleUpdate(frames_original * (1 + delay));
            }
        } else {
            curr_idx += 1;
            //ani_timer->start(1000 / fps);
            scheduleUpdate(frames_original);
        }
    } else {
        if (curr_idx == 0) {
            // reverse only occurs on ping pong mode so no need to check for that here
            forwards = true;
            //ani_timer->start(1000 / fps * (1 + delay));
            scheduleUpdate(frames_original * (1 + delay));
        } else {
            curr_idx -= 1;
            //ani_timer->start(1000 / fps);
            scheduleUpdate(frames_original);
        }
    }
}

unsigned AnimatedTile::getFrameCanonicalIndex(unsigned idx) {
    return animation_tiles.at(idx)->tile_id;
}

unsigned AnimatedTile::getAnimationLength() {
    return animation_tiles.size();
}

void AnimatedTile::scheduleUpdate(double frames_to_next) {
    int f_int = qRound(std::floor(frames_to_next));
    double f_rem = frames_to_next - f_int;

    frames_left += f_int;
    frames_remainder += f_rem;
    while (frames_remainder > 1) {
        frames_left++;
        frames_remainder -= 1;
    }
}

void AnimatedTile::advanceTimer() {
    if (fps == 0) {
        return;
    }
    if (frames_left > 0) {
        frames_left--;
    } else {
        updateTile();
    }
}
