#include "AnimationUser.h"
#include "../CarrotQt5.h"
#include "../graphics/ShaderSource.h"

AnimationUser::AnimationUser(std::shared_ptr<CarrotQt5> root) 
    : root(root), inTransition(false), cancellableTransition(false),
    currentAnimation({ nullptr, AnimState::IDLE, 0, sf::Sprite(),{ 0, 0, 0 } }),
    transition({ nullptr, AnimState::IDLE, 0, sf::Sprite(),{ 0, 0, 0 } }),
    animationTimer(-1l) {

}

AnimationUser::~AnimationUser() {

}

void AnimationUser::animationAdvance() {
    AnimationInstance& sourceAnim = (inTransition ? transition : currentAnimation);
    auto& source = sourceAnim.animation;

    int animationLength = source->frameCount;

    if (animationLength == 0) {
        setAnimation(AnimState::IDLE);
        return; // shouldn't happen
    }
    sourceAnim.frame = (sourceAnim.frame + 1) % (animationLength);
    if (sourceAnim.frame == 0 && inTransition) {
        inTransition = false;

        // Call the end hook; used e.g. for special move features for the player
        onTransitionEndHook();

        // Figure out the new sprite to use
        if (!inTransition) {
            sourceAnim = currentAnimation;
        } else {
            sourceAnim = transition;
        }
        auto& source = sourceAnim.animation;

        sourceAnim.sprite.setTexture(*(source->texture));
        sourceAnim.sprite.setOrigin(source->hotspot.x, source->hotspot.y);
    }

    int frameLeft = (sourceAnim.frame + source->frameOffset) * source->frameDimensions.x;
    int frameTop = (sourceAnim.frame / source->frameCount) * source->frameDimensions.y;

    sourceAnim.sprite.setTextureRect(sf::IntRect(frameLeft, frameTop, source->frameDimensions.x, source->frameDimensions.y));
}

void AnimationUser::loadAnimationSet(QMap<QString, std::shared_ptr<GraphicResource>>& animations) {
    animationBank = animations;
}

bool AnimationUser::setAnimation(AnimStateT state) {
    if (currentAnimation.animation != nullptr) {
        if ((currentAnimation.animation->state.contains(state)) || ((inTransition) && (!cancellableTransition))) {
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
        currentAnimation.frame = 0;
        currentAnimation.animation = candidates.at(0);
        currentAnimation.state = state;
        currentAnimation.sprite.setTexture(*(currentAnimation.animation->texture));
        currentAnimation.sprite.setTextureRect(
            sf::IntRect(currentAnimation.animation->frameOffset * currentAnimation.animation->frameDimensions.x, 0,
                currentAnimation.animation->frameDimensions.x, currentAnimation.animation->frameDimensions.y));
        currentAnimation.sprite.setOrigin(currentAnimation.animation->hotspot.x, currentAnimation.animation->hotspot.y);
        inTransition = false;
        transition.animation = nullptr;
    }
    cancelTimer(animationTimer);
    animationTimer = addTimer(static_cast<unsigned>(currentAnimation.animation->frameDuration / 1000.0 * 70.0),
        true, static_cast<TimerCallbackFunc>(&AnimationUser::animationAdvance));

    return true;
}

bool AnimationUser::setAnimation(const QString& animationId, const size_t& idx) {
    if (currentAnimation.animation != nullptr) {
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

    AnimationInstance& sourceAnim = (inTransition ? transition : currentAnimation);

    sf::RenderStates state;
    if (sourceAnim.color != sf::Vector3i(0, 0, 0)) {
        auto shader = ShaderSource::getShader("ColorizeShader").get();
        if (shader != nullptr) {
            shader->setParameter("color", sourceAnim.color.x / 255.0f, sourceAnim.color.y / 255.0f, sourceAnim.color.z / 255.0f);
            state.shader = shader;
        }
    }

    canvas->draw(sourceAnim.sprite, state);
}

bool AnimationUser::setAnimation(std::shared_ptr<GraphicResource> animation) {
    currentAnimation.frame = 0;
    currentAnimation.animation = animation;
    currentAnimation.sprite.setTexture(*(currentAnimation.animation->texture));
    currentAnimation.sprite.setTextureRect(sf::IntRect(
        currentAnimation.animation->frameOffset * currentAnimation.animation->frameDimensions.x, 0,
        currentAnimation.animation->frameDimensions.x, currentAnimation.animation->frameDimensions.y));
    currentAnimation.sprite.setOrigin(currentAnimation.animation->hotspot.x, currentAnimation.animation->hotspot.y);

    cancelTimer(animationTimer);
    animationTimer = addTimer(static_cast<unsigned>(currentAnimation.animation->frameDuration / 1000.0 * 70.0),
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
        transition.frame = 0;
        transition.animation = candidates.at(0);
        transition.state = state;
        transition.sprite.setTexture(*(transition.animation->texture));
        transition.sprite.setTextureRect(
            sf::IntRect(transition.animation->frameOffset * transition.animation->frameDimensions.x, 0,
                transition.animation->frameDimensions.x, transition.animation->frameDimensions.y));
        transition.sprite.setOrigin(transition.animation->hotspot.x, transition.animation->hotspot.y);
    }

    cancelTimer(animationTimer);
    animationTimer = addTimer(static_cast<unsigned>(transition.animation->frameDuration / 1000.0 * 70.0), 
        true, static_cast<TimerCallbackFunc>(&AnimationUser::animationAdvance));
    return true;
}

void AnimationUser::onTransitionEndHook() {
    // Called when a transition ends.
    // Objects should override this if they need to.
}
