#pragma once

#include <memory>

#include "../CommonActor.h"

class ActorAPI;
class GameView;
class Player;

class Collectible : public CommonActor {
public:
    Collectible(const ActorInstantiationDetails& initData, bool fromEventMap = true);
    ~Collectible();
    void tickEvent() override;
    void drawUpdate(std::shared_ptr<GameView>& view) override;
    virtual void collect(std::shared_ptr<Player> player);
    virtual void handleCollision(std::shared_ptr<CommonActor> other) override;

protected:
    bool untouched;
    uint scoreValue;
    double phase;
    void setFacingDirection();
};
