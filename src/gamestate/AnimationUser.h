#pragma once

#include <memory>
#include <QString>
#include <SFML/Graphics.hpp>
#include "TimerUser.h"
#include "../struct/AnimState.h"
#include "../struct/Resources.h"

class CarrotQt5;

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

    std::shared_ptr<GraphicResource> currentAnimation;
    AnimStateT currentState;
    std::shared_ptr<GraphicResource> transition;
    AnimStateT currentTransitionState;

    bool inTransition;
    bool cancellableTransition;
    unsigned frame;
    sf::Sprite sprite;
    sf::Vector3i color;
    std::shared_ptr<CarrotQt5> root;
    unsigned long animationTimer;
};
