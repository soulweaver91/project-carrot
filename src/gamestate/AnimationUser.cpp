#include "AnimationUser.h"
#include "../CarrotQt5.h"
#include "../graphics/ShaderSource.h"

AnimationUser::AnimationUser(std::shared_ptr<CarrotQt5> root) 
    : root(root), inTransition(false), cancellableTransition(false), animationTimer(-1l) {

}

AnimationUser::~AnimationUser() {

}

void AnimationUser::animationAdvance() {
    AnimationInstance& sourceAnim = (inTransition ? transition : currentAnimation);

    sourceAnim.advanceAnimation();
    if (sourceAnim.frame == 0 && inTransition) {
        inTransition = false;

        // Call the end hook; used e.g. for special move features for the player
        onTransitionEndHook();
    }
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
        currentAnimation.setAnimation(candidates.at(0));
        currentAnimation.state = state;
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
    sourceAnim.drawCurrentFrame(*canvas);
}

bool AnimationUser::setAnimation(std::shared_ptr<GraphicResource> animation) {
    currentAnimation.setAnimation(animation);

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
        transition.setAnimation(candidates.at(0));
        transition.state = state;
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

AnimationInstance::AnimationInstance() : animation(nullptr), state(AnimState::IDLE), frame(0), sprite(), color(0, 0, 0) {
}

void AnimationInstance::advanceAnimation() {
    int animationLength = animation->frameCount;

    if (animationLength == 0) {
        return;
    }

    frame = (frame + 1) % animationLength;

    int frameLeft = (frame + animation->frameOffset) * animation->frameDimensions.x;
    sprite.setTextureRect(sf::IntRect(frameLeft, 0, animation->frameDimensions.x, animation->frameDimensions.y));
}

void AnimationInstance::drawCurrentFrame(sf::RenderTarget& canvas) {
    sf::RenderStates state;
    if (color != sf::Vector3i(0, 0, 0)) {
        auto shader = ShaderSource::getShader("ColorizeShader").get();
        if (shader != nullptr) {
            shader->setParameter("color", color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
            state.shader = shader;
        }
    }

    canvas.draw(sprite, state);
}

void AnimationInstance::setAnimation(std::shared_ptr<GraphicResource> newAnimation) {
    animation = newAnimation;
    frame = 0;
    sprite.setTexture(*(newAnimation->texture));
    sprite.setTextureRect(sf::IntRect(0, 0,
        newAnimation->frameDimensions.x,
        newAnimation->frameDimensions.y));
    sprite.setPosition(-1, -1);
    sprite.setOrigin(newAnimation->hotspot.x, newAnimation->hotspot.y);
}

void AnimationInstance::setSpritePosition(const sf::Vector2f& position, const sf::Vector2f& scale) {
    sprite.setPosition(position);
    sprite.setScale(scale);
}
