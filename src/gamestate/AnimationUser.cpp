#include "AnimationUser.h"
#include "../CarrotQt5.h"

AnimationUser::AnimationUser(std::shared_ptr<CarrotQt5> root) 
    : root(root), inTransition(false), cancellableTransition(false), currentAnimation(nullptr), transition(nullptr),
    currentState(AnimState::IDLE), currentTransitionState(AnimState::IDLE) {

}

AnimationUser::~AnimationUser() {

}

void AnimationUser::animationAdvance() {
    std::shared_ptr<GraphicResource> source = (inTransition ? transition : currentAnimation);

    int anim_length = source->frameCount;

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
            source = currentAnimation;
        } else {
            source = transition;
        }
        sprite.setTexture(*(source->texture));
        sprite.setOrigin(source->hotspot.x, source->hotspot.y);
    }

    int frameleft = (frame + source->frameOffset) * source->frameDimensions.x;
    int frametop = (frame / source->frameCount)*source->frameDimensions.y;

    sprite.setTextureRect(sf::IntRect(frameleft, frametop, source->frameDimensions.x, source->frameDimensions.y));
}


/*size_t AnimationUser::addAnimation(AnimStateT state, const QString& filename, int frameCount, int frame_rows,
    int frame_width, int frame_height, int fps, int offset_x, int offset_y) {
    auto new_ani = std::make_shared<GraphicResource>();
    auto loaded_frames = root->requestTexture("Data/Assets/" + filename);

    if (loaded_frames != nullptr) {
        new_ani->texture = loaded_frames;
    } else {
        return -1;
    }
    new_ani->frameDuration = static_cast<int>(1000 / fps);
    new_ani->state = state;
    new_ani->frameCount = frameCount;
    new_ani->frame_rows = frame_rows;
    new_ani->frame_width = frame_width;
    new_ani->frame_height = frame_height;
    new_ani->offset_x = offset_x;
    new_ani->offset_y = offset_y;

    animationBank.push_back(new_ani);
    return animationBank.size() - 1;
}

size_t AnimationUser::assignAnimation(AnimStateT state, size_t original_idx) {
    if (animationBank.size() < original_idx) {
        // Invalid request
        return -1;
    }

    if (animationBank.at(original_idx)->state.contains(state)) {
        // No point in adding a second copy of a certain state-animation pair,
        // do nothing (but report the original index back for possible handling)
        return original_idx;
    }

    // Copy the requested state-animation pair and set a new state to it
    auto new_ani = std::make_shared<GraphicResource>(*animationBank.at(original_idx));
    new_ani->state = state;

    // Add the new pair to the animation bank and return its index
    animationBank.push_back(new_ani);
    return animationBank.size() - 1;
}*/

void AnimationUser::loadAnimationSet(QMap<QString, std::shared_ptr<GraphicResource>>& animations) {
    animationBank = animations;
}

bool AnimationUser::setAnimation(AnimStateT state) {
    if (currentAnimation != nullptr) {
        if ((currentAnimation->state.contains(state)) || ((inTransition) && (!cancellableTransition))) {
            return false;
        }
    }

    QList<std::shared_ptr<GraphicResource>> candidates;
    foreach(auto a, animationBank) {
        if (a->state.contains(state)) {
            candidates << a;
        }
    }

    if (candidates.size() == 0) {
        return false;
    } else {
        // get a random item later; uses first found for now
        frame = 0;
        currentAnimation = candidates.at(0);
        currentState = state;
        sprite.setTexture(*(currentAnimation->texture));
        sprite.setTextureRect(
            sf::IntRect(currentAnimation->frameOffset * currentAnimation->frameDimensions.x, 0, currentAnimation->frameDimensions.x, currentAnimation->frameDimensions.y));
        sprite.setOrigin(currentAnimation->hotspot.x, currentAnimation->hotspot.y);
        inTransition = false;
        transition = nullptr;
    }
    cancelTimer(animation_timer);
    animation_timer = addTimer(static_cast< unsigned >(currentAnimation->frameDuration / 1000.0 * 70.0),
        true, static_cast<TimerCallbackFunc>(&AnimationUser::animationAdvance));

    return true;
}

bool AnimationUser::setAnimation(const QString& animationId, const size_t& idx) {
    if (currentAnimation != nullptr) {
        if ((inTransition) && (!cancellableTransition)) {
            return false;
        }
    }

    if (animationBank.contains(animationId) && animationBank.count(animationId) > 0) {
        return setAnimation(animationBank.values(animationId)[idx]);
    }

    return true;
}

bool AnimationUser::setAnimation(std::shared_ptr<GraphicResource> animation) {
    frame = 0;
    currentAnimation = animation;
    sprite.setTexture(*(currentAnimation->texture));
    sprite.setTextureRect(sf::IntRect(currentAnimation->frameOffset * currentAnimation->frameDimensions.x, 0, currentAnimation->frameDimensions.x, currentAnimation->frameDimensions.y));
    sprite.setOrigin(currentAnimation->hotspot.x, currentAnimation->hotspot.y);

    cancelTimer(animation_timer);
    animation_timer = addTimer(static_cast< unsigned >(currentAnimation->frameDuration / 1000.0 * 70.0),
        true, static_cast<TimerCallbackFunc>(&AnimationUser::animationAdvance));
    return true;
}

bool AnimationUser::setTransition(AnimStateT state, bool cancellable) {
    QList<std::shared_ptr<GraphicResource>> candidates;
    foreach(auto a, animationBank) {
        if (a->state.contains(state)) {
            candidates << a;
        }
    }

    if (candidates.size() == 0) {
        return false;
    } else {
        inTransition = true;
        cancellableTransition = cancellable;
        frame = 0;
        transition = candidates.at(0);
        currentTransitionState = state;
        sprite.setTexture(*(transition->texture));
        sprite.setTextureRect(
            sf::IntRect(currentAnimation->frameOffset * currentAnimation->frameDimensions.x, 0, transition->frameDimensions.x, transition->frameDimensions.y));
        sprite.setOrigin(transition->hotspot.x, transition->hotspot.y);
    }
    cancelTimer(animation_timer);
    animation_timer = addTimer(static_cast< unsigned >(transition->frameDuration / 1000.0 * 70.0), 
        true, static_cast<TimerCallbackFunc>(&AnimationUser::animationAdvance));
    return true;
}

void AnimationUser::onTransitionEndHook() {
    // Called when a transition ends.
    // Objects should override this if they need to.
}
