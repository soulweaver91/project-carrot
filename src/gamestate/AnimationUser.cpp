#include "AnimationUser.h"
#include "../CarrotQt5.h"

AnimationUser::AnimationUser(std::shared_ptr<CarrotQt5> root) 
    : root(root), inTransition(false), cancellableTransition(false), current_animation(nullptr), transition(nullptr) {

}

AnimationUser::~AnimationUser() {

}

void AnimationUser::animationAdvance() {
    std::shared_ptr<StateAnimationPair> source = (inTransition ? transition : current_animation);

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
        sprite.setOrigin(source->offset_x, source->offset_y);
    }

    int frameleft = 0;
    if (source->frame_rows > 1) {
        frameleft = (frame % source->frame_cols)*source->frame_width;
    } else {
        frameleft = frame*source->frame_width;
    }
    int frametop = (frame / source->frame_cols)*source->frame_height;

    sprite.setTextureRect(sf::IntRect(frameleft, frametop, source->frame_width, source->frame_height));
}


size_t AnimationUser::addAnimation(ActorState state, const QString& filename, int frame_cols, int frame_rows,
    int frame_width, int frame_height, int fps, int offset_x, int offset_y) {
    auto new_ani = std::make_shared<StateAnimationPair>();
    auto loaded_frames = root->requestTexture("Data/Assets/" + filename);

    if (loaded_frames != nullptr) {
        new_ani->animation_frames = loaded_frames;
    } else {
        return -1;
    }
    new_ani->frametime = static_cast<int>(1000 / fps);
    new_ani->state = state;
    new_ani->frame_cols = frame_cols;
    new_ani->frame_rows = frame_rows;
    new_ani->frame_width = frame_width;
    new_ani->frame_height = frame_height;
    new_ani->offset_x = offset_x;
    new_ani->offset_y = offset_y;

    animation_bank.push_back(new_ani);
    return animation_bank.size() - 1;
}

size_t AnimationUser::assignAnimation(ActorState state, size_t original_idx) {
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
    auto new_ani = std::make_shared<StateAnimationPair>(*animation_bank.at(original_idx));
    new_ani->state = state;

    // Add the new pair to the animation bank and return its index
    animation_bank.push_back(new_ani);
    return animation_bank.size() - 1;
}

bool AnimationUser::setAnimation(ActorState state) {
    if (current_animation != nullptr) {
        if ((current_animation->state == state) || ((inTransition) && (!cancellableTransition))) {
            return false;
        }
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
            sf::IntRect(0, 0, current_animation->frame_width, current_animation->frame_height));
        sprite.setOrigin(current_animation->offset_x, current_animation->offset_y);
        inTransition = false;
        transition = nullptr;
    }
    cancelTimer(animation_timer);
    animation_timer = addTimer(static_cast< unsigned >(current_animation->frametime / 1000.0 * 70.0),
        true, static_cast<TimerCallbackFunc>(&AnimationUser::animationAdvance));

    return true;
}

bool AnimationUser::setAnimation(std::shared_ptr<StateAnimationPair> animation) {
    frame = 0;
    current_animation = animation;
    sprite.setTexture(*(current_animation->animation_frames));
    sprite.setTextureRect(sf::IntRect(0, 0, current_animation->frame_width, current_animation->frame_height));
    sprite.setOrigin(current_animation->offset_x, current_animation->offset_y);

    cancelTimer(animation_timer);
    animation_timer = addTimer(static_cast< unsigned >(current_animation->frametime / 1000.0 * 70.0),
        true, static_cast<TimerCallbackFunc>(&AnimationUser::animationAdvance));
    return true;
}

bool AnimationUser::setTransition(ActorState state, bool cancellable) {
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
            sf::IntRect(0, 0, transition->frame_width, transition->frame_height));
        sprite.setOrigin(transition->offset_x, transition->offset_y);
    }
    cancelTimer(animation_timer);
    animation_timer = addTimer(static_cast< unsigned >(transition->frametime / 1000.0 * 70.0), 
        true, static_cast<TimerCallbackFunc>(&AnimationUser::animationAdvance));
    return true;
}

void AnimationUser::onTransitionEndHook() {
    // Called when a transition ends.
    // Objects should override this if they need to.
}
