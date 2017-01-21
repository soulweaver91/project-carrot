#pragma once

#include <memory>

#include "SolidObject.h"

class ActorAPI;
class Player;

class PowerUpMonitor : public SolidObject {
public:
    PowerUpMonitor(const ActorInstantiationDetails& initData, WeaponType type);
    ~PowerUpMonitor();
    void handleCollision(std::shared_ptr<CommonActor> other) override;
    bool perish() override;
    void destroyAndApplyToPlayer(std::shared_ptr<Player> player, int strength);

private:
    WeaponType type;
};