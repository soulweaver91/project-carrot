#pragma once

#include <memory>
#include <QString>
#include <SFML/Graphics.hpp>
#include "TimerUser.h"
#include "../struct/AnimState.h"
#include "../struct/Resources.h"

class CarrotQt5;

class AnimationInstance {
public:
    AnimationInstance();
    std::shared_ptr<GraphicResource> animation;
    unsigned frame;
    sf::Vector3i color;
    void advanceAnimation();
    void drawCurrentFrame(sf::RenderTarget& target);
    void setAnimation(std::shared_ptr<GraphicResource> newAnimation, const AnimStateT& newState = AnimState::IDLE);
    void setSpritePosition(const sf::Vector2f& position, const sf::Vector2f& scale = { 1.0, 1.0 });
    const AnimStateT getAnimationState();

private:
    sf::Sprite sprite;
    AnimStateT state;
};

class AnimationUser : public TimerUser {
public:
    AnimationUser(std::shared_ptr<CarrotQt5> root);
    ~AnimationUser();

    void loadAnimationSet(QMap<QString, std::shared_ptr<GraphicResource>>& animations);
    virtual bool setAnimation(AnimStateT state);
    virtual bool setTransition(AnimStateT state, bool cancellable);

protected:
    void animationAdvance();
    virtual void onTransitionEndHook();
    bool setAnimation(std::shared_ptr<GraphicResource> animation);
    bool setAnimation(const QString& animationID, const size_t& idx = 0);
    void drawCurrentFrame();

    QMap<QString, std::shared_ptr<GraphicResource>> animationBank;

    AnimationInstance currentAnimation;
    AnimationInstance transition;

    bool inTransition;
    bool cancellableTransition;
    std::shared_ptr<CarrotQt5> root;
    unsigned long animationTimer;
};
