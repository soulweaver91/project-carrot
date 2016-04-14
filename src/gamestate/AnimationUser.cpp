#include "AnimationUser.h"
#include "../CarrotQt5.h"
#include "../graphics/ShaderSource.h"

AnimationUser::AnimationUser(std::shared_ptr<CarrotQt5> root) 
    : root(root), inTransition(false), cancellableTransition(false), currentAnimation(nullptr), transition(nullptr),
    animationTimer(-1l), currentState(AnimState::IDLE), currentTransitionState(AnimState::IDLE), color({ 0, 0, 0 }) {

}

AnimationUser::~AnimationUser() {

}

void AnimationUser::animationAdvance() {
    std::shared_ptr<GraphicResource> source = (inTransition ? transition : currentAnimation);

    int animationLength = source->frameCount;

    if (animationLength == 0) {
        setAnimation(AnimState::IDLE);
        return; // shouldn't happen
    }
    frame = (frame + 1) % (animationLength);
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

    int frameLeft = (frame + source->frameOffset) * source->frameDimensions.x;
    int frameTop = (frame / source->frameCount) * source->frameDimensions.y;

    sprite.setTextureRect(sf::IntRect(frameLeft, frameTop, source->frameDimensions.x, source->frameDimensions.y));
}

void AnimationUser::loadAnimationSet(QMap<QString, std::shared_ptr<GraphicResource>>& animations) {
    animationBank = animations;
}

bool AnimationUser::setAnimation(AnimStateT state) {
    if (currentAnimation != nullptr) {
        if ((currentAnimation->state.contains(state)) || ((inTransition) && (!cancellableTransition))) {
            return false;
        }
    }

    QVector<std::shared_ptr<GraphicResource>> candidates;
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
            sf::IntRect(currentAnimation->frameOffset * currentAnimation->frameDimensions.x, 0,
                currentAnimation->frameDimensions.x, currentAnimation->frameDimensions.y));
        sprite.setOrigin(currentAnimation->hotspot.x, currentAnimation->hotspot.y);
        inTransition = false;
        transition = nullptr;
    }
    cancelTimer(animationTimer);
    animationTimer = addTimer(static_cast<unsigned>(currentAnimation->frameDuration / 1000.0 * 70.0),
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

void AnimationUser::drawCurrentFrame() {
    auto canvas = root->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    sf::RenderStates state;
    if (color != sf::Vector3i(0, 0, 0)) {
        auto shader = ShaderSource::getShader("ColorizeShader").get();
        if (shader != nullptr) {
            shader->setParameter("color", color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
            state.shader = shader;
        }
    }

    canvas->draw(sprite, state);
}

bool AnimationUser::setAnimation(std::shared_ptr<GraphicResource> animation) {
    frame = 0;
    currentAnimation = animation;
    sprite.setTexture(*(currentAnimation->texture));
    sprite.setTextureRect(sf::IntRect(currentAnimation->frameOffset * currentAnimation->frameDimensions.x, 0,
        currentAnimation->frameDimensions.x, currentAnimation->frameDimensions.y));
    sprite.setOrigin(currentAnimation->hotspot.x, currentAnimation->hotspot.y);

    cancelTimer(animationTimer);
    animationTimer = addTimer(static_cast<unsigned>(currentAnimation->frameDuration / 1000.0 * 70.0),
        true, static_cast<TimerCallbackFunc>(&AnimationUser::animationAdvance));
    return true;
}

bool AnimationUser::setTransition(AnimStateT state, bool cancellable) {
    QVector<std::shared_ptr<GraphicResource>> candidates;
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
            sf::IntRect(currentAnimation->frameOffset * currentAnimation->frameDimensions.x, 0,
                transition->frameDimensions.x, transition->frameDimensions.y));
        sprite.setOrigin(transition->hotspot.x, transition->hotspot.y);
    }

    cancelTimer(animationTimer);
    animationTimer = addTimer(static_cast<unsigned>(transition->frameDuration / 1000.0 * 70.0), 
        true, static_cast<TimerCallbackFunc>(&AnimationUser::animationAdvance));
    return true;
}

void AnimationUser::onTransitionEndHook() {
    // Called when a transition ends.
    // Objects should override this if they need to.
}
