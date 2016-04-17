#pragma once

#include <memory>
#include <QString>
#include <SFML/Graphics.hpp>
#include "TimerUser.h"
#include "../struct/AnimState.h"
#include "../struct/Resources.h"

class CarrotQt5;
class AnimationInstance;
class AnimationUser;

typedef void(AnimationUser::*AnimationCallbackFunc)(std::shared_ptr<AnimationInstance>);

class AnimationInstance : public TimerUser, public std::enable_shared_from_this<AnimationInstance> {
public:
    AnimationInstance(AnimationUser* const owner);
    std::shared_ptr<GraphicResource> animation;
    void drawCurrentFrame(sf::RenderTarget& target);
    void setAnimation(std::shared_ptr<GraphicResource> newAnimation, const AnimStateT& newState = AnimState::IDLE,
        AnimationCallbackFunc cb = nullptr);
    void setSpritePosition(const sf::Vector2f& position, const sf::Vector2f& scale = { 1.0, 1.0 });
    const AnimStateT getAnimationState();
    void clearCallback();
    void resetFrame();
    void setColor(const sf::Vector3i& newColor);

private:
    void advanceAnimation();
    void doCallback();
    unsigned frame;
    sf::Sprite sprite;
    AnimStateT state;
    sf::Vector3i color;
    unsigned long animationTimer;
    AnimationCallbackFunc callback;
    AnimationUser* const owner;
};

class AnimationUser : public TimerUser {
public:
    AnimationUser(std::shared_ptr<CarrotQt5> root);
    ~AnimationUser();

    void loadAnimationSet(QMap<QString, std::shared_ptr<GraphicResource>>& animations);
    void advanceAnimationTimers();
    virtual bool setAnimation(AnimStateT state);
    virtual bool setTransition(AnimStateT state, bool cancellable, AnimationCallbackFunc callback = nullptr);
    virtual void animationFinishedHook(std::shared_ptr<AnimationInstance> animation);

protected:
    bool setAnimation(std::shared_ptr<GraphicResource> animation);
    bool setAnimation(const QString& animationID, const size_t& idx = 0);
    void drawCurrentFrame();

    QMap<QString, std::shared_ptr<GraphicResource>> animationBank;

    AnimationInstance currentAnimation;
    AnimationInstance transition;

    bool inTransition;
    bool cancellableTransition;
    std::shared_ptr<CarrotQt5> root;
};
