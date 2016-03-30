#include "CommonActor.h"

#include "CarrotQt5.h"
#include "TileMap.h"
#include "EventMap.h"

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <QTimer>

CommonActor::CommonActor(CarrotQt5* game_root, double x, double y, bool fromEventMap) : root(game_root),
    speed_h(0), speed_v(0), thrust(0), push(0), canJump(false), facingLeft(false),
    inTransition(false), cancellableTransition(false), max_health(1), health(1), isGravityAffected(true),
    isClippingAffected(true), elasticity(0.0), isInvulnerable(false), friction(root->gravity/3),
    isBlinking(false), isSuspended(false), pos_x(x), pos_y(y), destWindow(root->window),
    current_animation(nullptr), transition(nullptr), createdFromEventMap(fromEventMap), animation_timer(-1l), next_timer(0) {
    origin_x = static_cast<int>(x) / 32;
    origin_y = static_cast<int>(y) / 32;
}

CommonActor::~CommonActor() {
    root->removeActor(this);
    for (int i = animation_bank.size() - 1; i >= 0; --i) {
        delete animation_bank.at(i);
    }
}

void CommonActor::DrawUpdate() {
    // Don't draw anything if we aren't in the vicinity of the view
    sf::Vector2f view = root->window->getView().getCenter();
    if ((std::abs(view.x - pos_x) > (root->getViewWidth() / 2) + 50)
     || (std::abs(view.y - pos_y) > (root->getViewHeight() / 2) + 50)) {
        return;
    }

    if (root->dbgShowMasked) {
        double len = sqrt(speed_h*speed_h + speed_v*speed_v);
        if (len > 0) {
            sf::RectangleShape line(sf::Vector2f(len * 4 + 5, 5));
            line.setPosition(pos_x,pos_y);
            line.setOrigin(2, 2);
            double ang = atan2(speed_v,speed_h);
            line.setRotation(180 + ang * 180 / 3.1415926535);
            line.setFillColor(sf::Color(255,255,0));
            root->window->draw(line);
        }
    }
    
    if (!((isBlinking) && ((root->getFrame() % 6) > 2))) {
        // Pick the appropriate animation depending on if we are in the midst of a transition
        StateAnimationPair* source = (inTransition ? transition : current_animation);
    
        sprite.setScale((facingLeft ? -1 : 1),1);
        sprite.setPosition(pos_x,pos_y);
        destWindow->draw(sprite);
    }
}

void CommonActor::animationAdvance() {
    StateAnimationPair* source = (inTransition ? transition : current_animation);

    int anim_length = source->frame_cols * source->frame_rows;

    if (anim_length == 0) {
        setAnimation(AnimState::IDLE);
        return; // shouldn't happen
    }
    frame = (frame + 1) % (anim_length);
    if (frame == 0 && inTransition) {
        inTransition = false;

        // Call the end hook; used e.g. for special move features for the player
        onTransitionEndHook();

        // Figure out the new sprite to use
        if (!inTransition) {
            source = current_animation;
        } else {
            source = transition;
        }
        sprite.setTexture(*(source->animation_frames));
        sprite.setOrigin(source->offset_x,source->offset_y);
    }

    int frameleft = 0;
    if (source->frame_rows > 1) {
        frameleft = (frame % source->frame_cols)*source->frame_width;
    } else {
        frameleft = frame*source->frame_width;
    }
    int frametop = (frame / source->frame_cols)*source->frame_height;

    sprite.setTextureRect(sf::IntRect(frameleft,frametop,source->frame_width,source->frame_height));
}

void CommonActor::keyPressEvent(QKeyEvent* event) {
    // nothing to do in this event unless a child class
    // overrides the function
}

void CommonActor::keyReleaseEvent(QKeyEvent* event) {
    // nothing to do in this event unless a child class
    // overrides the function
}

void CommonActor::tickEvent() {
    // Sign of the speed: either -1, 0 or 1
    short sign = ((speed_h + push) > 1e-6) ? 1 : (((speed_h + push) < -1e-6) ? -1 : 0);
    double gravity = (isGravityAffected ? root->gravity : 0);
   
    speed_h = std::min(std::max(speed_h, -16.0),16.0);
    speed_v = std::min(std::max(speed_v + gravity - thrust, -16.0),16.0);

    Hitbox here = getHitbox();
    if (!root->isPositionEmpty(CarrotQt5::calcHitbox(here,speed_h + push,speed_v), speed_v > 0,this)) {
        if (abs(speed_h + push) > 1e-6) {
            // We are walking, thus having both vertical and horizontal speed
            if (root->isPositionEmpty(CarrotQt5::calcHitbox(here,speed_h + push,0), speed_v > 0,this)) {
                // We could go toward the horizontal direction only
                // Chances are we're just casually strolling and gravity tries to pull us through,
                // or we are falling diagonally and hit a floor
                if (speed_v > 0) {
                    // Yep, that's it; just negate the gravity effect
                    speed_v = -(elasticity * speed_v);
                    onHitFloorHook();
                    canJump = true;
                } else {
                    // Nope, hit a wall from below diagonally then. Let's bump back a bit
                    speed_v = 1;
                    thrust = 0;
                    canJump = false;
                    onHitCeilingHook();
                }
            } else {
                // Nope, there's also some obstacle horizontally
                // Let's figure out if we are going against an upward slope
                if (root->isPositionEmpty(CarrotQt5::calcHitbox(here,speed_h + push,-abs(speed_h + push)-5), false, this)) {
                    // Yes, we indeed are
                    speed_v = -(elasticity * speed_v);
                    canJump = true;
                    pos_y -= abs(speed_h + push)+1;
                    /*while (root->game_tiles->isTileEmpty(CarrotQt5::calcHitbox(getHitbox(),speed_h,speed_v-abs(speed_h)-2))) {
                        pos_y += 0.5;
                    }
                    pos_y -= 1;*/
                    
                    // TODO: position us vertically to a suitable position so that we won't raise up to the air needlessly
                    // (applicable if the slope isn't exactly at 45 degrees)
                } else {
                    // Nope. Cannot move horizontally at all. Can we just go vertically then?
                    speed_h = -(elasticity * speed_h);
                    push *= -1;
                    if (root->isPositionEmpty(CarrotQt5::calcHitbox(here,0,speed_v), speed_v > 0, this)) {
                        // Yeah
                        canJump = false;
                        onHitWallHook();
                    } else {
                        // Nope
                        if (speed_v > 0) {
                            canJump = true;
                        }

                        speed_v = -(elasticity * speed_v);
                        thrust = 0;
                        // TODO: fix a problem with hurt getting player stuck in a wall here
                        onHitWallHook();
                        onHitFloorHook();
                    }
                }
            }
        } else {
            // We are going directly vertically
            if (speed_v > 0) {
                // We are falling, or we are on solid ground and gravity tries to push us through the floor
                if (root->isPositionEmpty(CarrotQt5::calcHitbox(here,0,0), true, this)) {
                    // Let's just nullify that effect
                    speed_v = -(elasticity * speed_v);
                    while (root->isPositionEmpty(CarrotQt5::calcHitbox(getHitbox(),speed_h,speed_v),true,this)) {
                        pos_y += 0.5;
                    }
                    pos_y -= 0.5;
                    
                    onHitFloorHook();
                    canJump = true;
                } else {
                    // Nope, nothing else is going on
                    speed_v = -(elasticity * speed_v);
                    canJump = true;
                }
            } else {
                // We are jumping
                if (!root->isPositionEmpty(CarrotQt5::calcHitbox(here,0,speed_v), false, this)) {
                    speed_v = -(elasticity * speed_v);
                    thrust = 0;
                    onHitCeilingHook();
                } else {
                    // we can go vertically anyway
                    //speed_v = -(elasticity * speed_v);
                }
            }
        }
    } else {
        if (canJump) {
            // Check if we are running on a downhill slope. If so, keep us attached to said slope instead of flying off.
            if (!root->isPositionEmpty(CarrotQt5::calcHitbox(here,speed_h + push,speed_v+abs(speed_h + push)+5), false, this)) {
                while (root->isPositionEmpty(CarrotQt5::calcHitbox(getHitbox(),speed_h + push,speed_v+abs(speed_h + push)), false, this)) {
                    pos_y += 0.1;
                }
                pos_y -= 0.1;
            } else {
                // That wasn't the case so forget about that
                canJump = false;
            }
        }
    }
    
    if (std::abs(push) > 1e-6) {
        // Reduce remaining push
        if (push > 0) {
            push = std::max(push - friction,0.0);
        } else {
            push = std::min(push + friction,0.0);
        }
    }
    thrust = std::max(thrust - gravity/3,0.0);
    //speed_h = sign * std::max((sign * speed_h) - friction, 0.0);

    pos_x += speed_h + push;
    pos_y += speed_v;

    // determine current animation last bits from speeds
    // it's okay to call setAnimation on every tick because it doesn't do
    // anything if the animation is the same as it was earlier

    // only certain ones don't need to be preserved from earlier state, others should be set as expected
    if (current_animation != nullptr) {
        int composite = current_animation->state & 0xFFFFFFE0;
        if (abs(speed_h) > 3) {
            // shift-running, speed is more than 3px/frame
            composite += 3;
        } else if (abs(speed_h) > 1) {
            // running, speed is between 1px and 3px/frame
            composite += 2;
        } else if (abs(speed_h) > 1e-6) {
            // walking, speed is less than 1px/frame (mostly a transition zone)
            composite += 1;
        }
    
        if (isSuspended) {
            composite += 12;
        } else {
            if (canJump) {
                // grounded, no vertical speed
            } else if (speed_v > 1e-6) {
                // falling, ver. speed is positive
                composite += 8;
            } else if (speed_v < -1e-6) {
                // jumping, ver. speed is negative
                composite += 4;
            }
        }
    
        ActorState oldstate = current_animation->state;
        ActorState newstate = ActorState(composite);
        setAnimation(newstate);
    }

    // Make sure we stay within the level boundaries
    pos_x = std::min(std::max(pos_x,0.0),root->getLevelWidth()*32.0);
    pos_y = std::min(std::max(pos_y,0.0),root->getLevelHeight()*32.0);
} 

size_t CommonActor::addAnimation(ActorState state, const QString& filename, int frame_cols, int frame_rows,
                               int frame_width, int frame_height, int fps, int offset_x, int offset_y) {
    StateAnimationPair* new_ani = new StateAnimationPair();
    sf::Texture* loaded_frames = root->getCachedTexture("Data/Assets/" + filename);

    if (loaded_frames != nullptr) {
        new_ani->animation_frames = loaded_frames;
    } else {
        return -1;
    }
    new_ani->frametime = static_cast<int>(1000/fps);
    new_ani->state = state;
    new_ani->frame_cols = frame_cols;
    new_ani->frame_rows = frame_rows;
    new_ani->frame_width = frame_width;
    new_ani->frame_height = frame_height;
    new_ani->offset_x = offset_x;
    new_ani->offset_y = offset_y;

    animation_bank.push_back(new_ani);
    return animation_bank.size()-1;
}

size_t CommonActor::assignAnimation(ActorState state, size_t original_idx) {
    if (animation_bank.size() < original_idx) {
        // Invalid request
        return -1;
    }

    if (animation_bank.at(original_idx)->state == state) {
        // No point in adding a second copy of a certain state-animation pair,
        // do nothing (but report the original index back for possible handling)
        return original_idx;
    }

    // Copy the requested state-animation pair and set a new state to it
    StateAnimationPair* new_ani = new StateAnimationPair(*(animation_bank.at(original_idx)));
    new_ani->state = state;

    // Add the new pair to the animation bank and return its index
    animation_bank.push_back(new_ani);
    return animation_bank.size()-1;
}

bool CommonActor::setAnimation(ActorState state) {
    ActorState oldstate = AnimState::IDLE;
    if (current_animation != nullptr) {
        if ((current_animation->state == state) || ((inTransition) && (!cancellableTransition))) {
            return true;
        }
        oldstate = current_animation->state;
    }

    QList< int > candidates;
    for (unsigned i = 0; i < animation_bank.size(); ++i) {
        if (animation_bank.at(i)->state == state) {
            candidates << i;
        }
    }
    if (candidates.size() == 0) {
        return false;
    } else {
        // get a random item later; uses first found for now
        frame = 0;
        current_animation = animation_bank.at(candidates.at(0));
        sprite.setTexture(*(current_animation->animation_frames));
        sprite.setTextureRect(
            sf::IntRect(0,0,current_animation->frame_width,current_animation->frame_height));
        sprite.setOrigin(current_animation->offset_x,current_animation->offset_y);
        inTransition = false;
        transition = nullptr;
    }
    cancelTimer(animation_timer);
    animation_timer = addTimer(static_cast< unsigned >(current_animation->frametime / 1000.0 * 70.0),true,&CommonActor::animationAdvance);
    
    ActorState newstate = state;

    switch(oldstate) {
        case AnimState::RUN:
            if ((newstate == AnimState::IDLE) || (newstate == AnimState::WALK)) {
                setTransition(AnimState::TRANSITION_RUN_TO_IDLE,true);
            }
            if (newstate == AnimState::DASH) {
                setTransition(AnimState::TRANSITION_RUN_TO_DASH,true);
            }
            break;
        case AnimState::FALL:
            if (newstate == AnimState::IDLE) {
                setTransition(AnimState::TRANSITION_IDLE_FALL_TO_IDLE,true);
            }
            break;
        case AnimState::IDLE:
            if (newstate == AnimState::JUMP) {
                setTransition(AnimState::TRANSITION_IDLE_TO_IDLE_JUMP,true);
            }
            break;
        case AnimState::SHOOT:
            if (newstate == AnimState::IDLE) {
                setTransition(AnimState::TRANSITION_IDLE_SHOOT_TO_IDLE,true);
            }
            break;
    }
    return true;
}

bool CommonActor::setAnimation(StateAnimationPair* animation) {
    frame = 0;
    current_animation = animation;
    sprite.setTexture(*(current_animation->animation_frames));
    sprite.setTextureRect(sf::IntRect(0,0,current_animation->frame_width,current_animation->frame_height));
    sprite.setOrigin(current_animation->offset_x,current_animation->offset_y);
        
    cancelTimer(animation_timer);
    animation_timer = addTimer(static_cast< unsigned >(current_animation->frametime / 1000.0 * 70.0),true,&CommonActor::animationAdvance);
    return true;
}

bool CommonActor::setTransition(ActorState state, bool cancellable) {
    QList< int > candidates;
    for (unsigned i = 0; i < animation_bank.size(); ++i) {
        if (animation_bank.at(i)->state == state) {
            candidates << i;
        }
    }
    if (candidates.size() == 0) {
        return false;
    } else {
        inTransition = true;
        cancellableTransition = cancellable;
        frame = 0;
        transition = animation_bank.at(candidates.at(0));
        sprite.setTexture(*(transition->animation_frames));
        sprite.setTextureRect(
            sf::IntRect(0,0,transition->frame_width,transition->frame_height));
        sprite.setOrigin(transition->offset_x,transition->offset_y);
    }
    cancelTimer(animation_timer);
    animation_timer = addTimer(static_cast< unsigned >(transition->frametime / 1000.0 * 70.0),true,&CommonActor::animationAdvance);
    return true;
}

void CommonActor::setToViewCenter(sf::View* view) {
    view->setCenter(
        std::max(400.0,std::min(32.0 * (root->getLevelWidth()+1)  - 400.0, (double)qRound(pos_x))),
        std::max(300.0,std::min(32.0 * (root->getLevelHeight()+1) - 300.0, (double)qRound(pos_y)))
    );
}

CoordinatePair CommonActor::getPosition() {
    CoordinatePair pos_now = {pos_x, pos_y};
    return pos_now;
}

Hitbox CommonActor::getHitbox() {
    if (current_animation != nullptr) {
        Hitbox box = {pos_x - current_animation->offset_x,
                      pos_y - current_animation->offset_y,
                      pos_x - current_animation->offset_x + current_animation->frame_width,
                      pos_y - current_animation->offset_y + current_animation->frame_height
        };
        return box;
    }
    Hitbox box = {0,0,0,0};
    return box;
}

void CommonActor::removeInvulnerability() {
    isInvulnerable = false;
    isBlinking = false;
}

void CommonActor::decreaseHealth(unsigned amount) {
    if (amount > health) {
        health = 0;
        return;
    }
    health -= amount;
}

bool CommonActor::perish() {
    // Delayed deletion routine for all actors, run after all actors' tick events
    // If health drops to 0, delete self and remove spawner from event map
    // Can be overridden, e.g. the player object instead implements death routines
    // like loading last save point here
    if (health == 0) {
        root->game_events->deactivate(origin_x, origin_y);
        root->game_events->storeTileEvent(origin_x, origin_y, PC_EMPTY);
        delete this;
        return true;
    }
    return false;
}

void CommonActor::onHitFloorHook() {
    // Called from inside the position update code when the object hits floor
    // and was falling earlier. Objects should override this if they need to
    // (e.g. the Player class playing a sound).
}

void CommonActor::onHitCeilingHook() {
    // Called from inside the position update code when the object hits ceiling.
    // Objects should override this if they need to.
}

void CommonActor::onHitWallHook() {
    // Called from inside the position update code when the object hits a wall.
    // Objects should override this if they need to.
}

void CommonActor::onTransitionEndHook() {
    // Called when a transition ends.
    // Objects should override this if they need to.
}

bool CommonActor::deactivate(int x, int y, int dist) {
    if ((std::abs(x - origin_x) > dist) || (std::abs(y - origin_y) > dist)) {
        root->game_events->deactivate(origin_x,origin_y);
        delete this;
        return true;
    }
    return false;
}

void CommonActor::moveInstantly(CoordinatePair location) {
    pos_x = location.x;
    pos_y = location.y;
}

void CommonActor::deleteFromEventMap() {
    root->game_events->storeTileEvent(origin_x, origin_y, PC_EMPTY);
}

void CommonActor::advanceTimers() {
    for (int i = 0; i < timers.size(); ++i) {
        timers[i].second.frames_left--;
        if (timers[i].second.frames_left == 0) {
            invokeTimer(i);
            if (timers[i].second.recurring) {
                timers[i].second.frames_remainder += timers[i].second.frames_original_remainder;
                while (timers[i].second.frames_remainder > 1) {
                    timers[i].second.frames_remainder -= 1.0;
                    timers[i].second.frames_left++;
                }
                timers[i].second.frames_left += timers[i].second.frames_original;
            } else {
                timers.removeAt(i);
                --i;
            }
        }
    }
}

void CommonActor::invokeTimer(int idx) {
    (this->*(timers[idx].second.func))();
}


unsigned long CommonActor::addTimer(unsigned frames, bool recurring, ActorFunc func) {
    ActorTimer t = {frames, 0.0, frames, 0.0, recurring, func};
    timers.append(qMakePair(next_timer,t));
    return next_timer++;
}
unsigned long CommonActor::addTimer(double frames, bool recurring, ActorFunc func) {
    unsigned long floored = qRound(floor(frames));
    
    ActorTimer t = {floored, frames - floored, floored, frames - floored, recurring, func};
    timers.append(qMakePair(next_timer,t));
    return next_timer++;
}

void CommonActor::cancelTimer(unsigned long idx) {
    for (int i = 0; i < timers.size(); ++i) {
        if (timers[i].first == idx) {
            timers.removeAt(i);
            return;
        }
    }
}

