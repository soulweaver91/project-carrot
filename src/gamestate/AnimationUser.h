#pragma once

#include <memory>
#include <functional>
#include <QString>
#include <SFML/Graphics.hpp>
#include "TimerUser.h"
#include "LevelManager.h"
#include "../struct/AnimState.h"
#include "../struct/Resources.h"

class ActorAPI;
class AnimationInstance;
class AnimationUser;
class GameView;

typedef std::function<void()> AnimationCallbackFunc;

class AnimationInstance : public TimerUser, public std::enable_shared_from_this<AnimationInstance> {
public:
    AnimationInstance(AnimationUser* const owner);
    void drawCurrentFrame(sf::RenderTarget& target);
    void setAnimation(std::shared_ptr<GraphicResource> newAnimation, const AnimStateT& newState = AnimState::IDLE,
        AnimationCallbackFunc cb = []() {});
    void setSpritePosition(const sf::Vector2f& position, const sf::Vector2f& scale = { 1.0, 1.0 });
    AnimStateT getAnimationState();
    std::shared_ptr<const GraphicResource> getAnimation();
    void clearCallback();
    void resetFrame(int idx = -1);
    void resetAnimation();
    void setColor(const sf::Vector3i& newColor);
    sf::Vector3i getColor();
    ActorGraphicState getGraphicState();

private:
    void advanceAnimation();
    void doCallback();

    std::shared_ptr<GraphicResource> animation;
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
    AnimationUser(std::shared_ptr<ActorAPI> api);
    ~AnimationUser();

    void loadAnimationSet(QMap<QString, std::shared_ptr<GraphicResource>>& animations);
    void advanceAnimationTimers();
    virtual bool setAnimation(AnimStateT state);
    virtual bool setTransition(AnimStateT state, bool cancellable, AnimationCallbackFunc callback = []() {});
    virtual void animationFinishedHook(std::shared_ptr<AnimationInstance> animation);

protected:
    bool setAnimation(std::shared_ptr<GraphicResource> animation);
    bool setAnimation(const QString& animationID, const size_t& idx = 0);
    QVector<std::shared_ptr<GraphicResource>> findAnimationCandidates(const AnimStateT& state);
    void drawCurrentFrame(std::shared_ptr<GameView>& view);

    QMap<QString, std::shared_ptr<GraphicResource>> animationBank;

    std::shared_ptr<AnimationInstance> currentAnimation;
    std::shared_ptr<AnimationInstance> transition;

    std::shared_ptr<ActorAPI> api;
    bool inTransition;
    bool cancellableTransition;
};
