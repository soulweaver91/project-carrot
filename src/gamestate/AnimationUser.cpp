#include "AnimationUser.h"
#include "../CarrotQt5.h"
#include "../gamestate/GameView.h"
#include "../graphics/ShaderSource.h"

AnimationUser::AnimationUser(std::shared_ptr<CarrotQt5> root) 
    : root(root), inTransition(false), cancellableTransition(false) {
    currentAnimation = std::make_shared<AnimationInstance>(this);
    transition = std::make_shared<AnimationInstance>(this);
}

AnimationUser::~AnimationUser() {

}

void AnimationUser::loadAnimationSet(QMap<QString, std::shared_ptr<GraphicResource>>& animations) {
    animationBank = animations;
}

void AnimationUser::advanceAnimationTimers() {
    std::shared_ptr<AnimationInstance> sourceAnim = (inTransition ? transition : currentAnimation);
    sourceAnim->advanceTimers();
}

void AnimationUser::animationFinishedHook(std::shared_ptr<AnimationInstance> animation) {
    if (inTransition) {
        inTransition = false;
        currentAnimation->resetFrame();
    }
}

bool AnimationUser::setAnimation(AnimStateT state) {
    if ((currentAnimation->getAnimation()->state.contains(state)) || ((inTransition) && (!cancellableTransition))) {
        return false;
    }

    auto candidates = findAnimationCandidates(state);
    if (candidates.size() == 0) {
        return false;
    } else {
        currentAnimation->setAnimation(candidates.at(qrand() % candidates.size()), state);
        if (inTransition) {
            inTransition = false;
            transition->resetAnimation();
        }
    }

    return true;
}

bool AnimationUser::setAnimation(const QString& animationId, const size_t& idx) {
    if ((inTransition) && (!cancellableTransition)) {
        return false;
    }

    if (animationBank.contains(animationId) && animationBank.count(animationId) > 0) {
        return setAnimation(animationBank.values(animationId)[idx]);
    }

    return true;
}

QVector<std::shared_ptr<GraphicResource>> AnimationUser::findAnimationCandidates(const AnimStateT& state) {
    QVector<std::shared_ptr<GraphicResource>> candidates;
    for (const auto& a : animationBank) {
        if (a->state.contains(state)) {
            candidates << a;
        }
    }

    return candidates;
}

void AnimationUser::drawCurrentFrame(std::shared_ptr<GameView>& view) {
    auto canvas = view->getCanvas().lock();
    if (canvas == nullptr) {
        return;
    }

    std::shared_ptr<AnimationInstance> sourceAnim = (inTransition ? transition : currentAnimation);
    sourceAnim->drawCurrentFrame(*canvas);
}

bool AnimationUser::setAnimation(std::shared_ptr<GraphicResource> animation) {

    AnimStateT state = AnimState::IDLE;
    if (animation->state.size() > 0) {
        state = animation->state.values().at(0);
    }
    currentAnimation->setAnimation(animation, state);

    return true;
}

bool AnimationUser::setTransition(AnimStateT state, bool cancellable, AnimationCallbackFunc callback) {
    auto candidates = findAnimationCandidates(state);
    if (candidates.size() == 0) {
        if (callback != nullptr) {
            (this->*(callback))(nullptr);
        }

        return false;
    } else {
        inTransition = true;
        cancellableTransition = cancellable;
        transition->setAnimation(candidates.at(0), state, callback);
    }

    return true;
}

AnimationInstance::AnimationInstance(AnimationUser* const owner) : animation(nullptr), state(AnimState::STATE_UNINITIALIZED),
    frame(0), sprite(), color(0, 0, 0), animationTimer(0), owner(owner) {
}

void AnimationInstance::advanceAnimation() {
    int animationLength = animation->frameCount;

    if (animationLength == 0) {
        owner->animationFinishedHook(nullptr);
        if (callback != nullptr) {
            doCallback();
        }
        return;
    }

    frame = (frame + 1) % animationLength;

    int frameLeft = ((frame + animation->frameOffset) % animation->frameConfiguration.x) * animation->frameDimensions.x;
    int frameTop  = ((frame + animation->frameOffset) / animation->frameConfiguration.x) * animation->frameDimensions.y;
    sprite.setTextureRect(sf::IntRect(frameLeft, frameTop, animation->frameDimensions.x, animation->frameDimensions.y));

    if (frame == 0) {
        owner->animationFinishedHook(nullptr);
        if (callback != nullptr) {
            doCallback();
        }
    }
}

void AnimationInstance::drawCurrentFrame(sf::RenderTarget& canvas) {
    sf::RenderStates state;
    if (color != sf::Vector3i(0, 0, 0)) {
        auto shader = ShaderSource::getShader("ColorizeShader").get();
        shader->setParameter("color", color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
        state.shader = shader;
    }

    canvas.draw(sprite, state);
}

void AnimationInstance::setAnimation(std::shared_ptr<GraphicResource> newAnimation,
    const AnimStateT& newState, AnimationCallbackFunc cb) {
    animation = newAnimation;
    state = newState;
    callback = cb;
    sprite.setTexture(*(newAnimation->texture));
    resetFrame();
    sprite.setPosition(-1, -1);
    sprite.setOrigin(newAnimation->hotspot.x, newAnimation->hotspot.y);
    cancelTimer(animationTimer);
    animationTimer = addTimer(static_cast<unsigned>(animation->frameDuration / 1000.0 * 70.0),
        true, [this]() {
        advanceAnimation();
    });
}

void AnimationInstance::setSpritePosition(const sf::Vector2f& position, const sf::Vector2f& scale) {
    sprite.setPosition(position);
    sprite.setScale(scale);
}

const AnimStateT AnimationInstance::getAnimationState() {
    return state;
}

std::shared_ptr<const GraphicResource> AnimationInstance::getAnimation() {
    if (animation != nullptr) {
        return animation;
    }

    return std::make_shared<GraphicResource>();
}

void AnimationInstance::clearCallback() {
    callback = nullptr;
}

void AnimationInstance::resetFrame() {
    frame = animation->frameOffset;
    sprite.setTextureRect(sf::IntRect(
        (frame % animation->frameConfiguration.x) * animation->frameDimensions.x,
        (frame / animation->frameConfiguration.x) * animation->frameDimensions.y,
        animation->frameDimensions.x,
        animation->frameDimensions.y));
}

void AnimationInstance::resetAnimation() {
    animation = nullptr;
    cancelTimer(animationTimer);
    if (callback != nullptr && owner != nullptr) {
        doCallback();
    }
}

void AnimationInstance::setColor(const sf::Vector3i & newColor) {
    color = newColor;
}

ActorGraphicState AnimationInstance::getGraphicState() {
    if (animation == nullptr) {
        return {
            std::make_shared<QBitArray>(1, false),
            { 0, 0 },
            { 1, 1 },
            sf::FloatRect(),
            { 1.0, 1.0 },
            { 0.0, 0.0 },
            0.0f
        };
    }
    return {
        animation->bitmasks.at(frame),
        animation->hotspot,
        animation->frameDimensions,
        sprite.getGlobalBounds(),
        { 1.0, 1.0 },
        { 0.0, 0.0 },
        0.0f
    };
}

void AnimationInstance::doCallback() {
    (owner->*(callback))(shared_from_this());
}
